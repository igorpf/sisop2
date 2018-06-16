#include <iostream>
#include <cstring>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/table_printer.hpp"
#include "../../util/include/File.hpp"
#include "../../util/include/lock_guard.hpp"
#include "../../util/include/LoggerFactory.hpp"

#include "../include/login_command_parser.hpp"

#include "../include/dropboxClient.hpp"

namespace fs = boost::filesystem;
namespace util = dropbox_util;

const std::string Client::LOGGER_NAME = "Client";

Client::Client() : logger_(LOGGER_NAME) {}

void Client::start_client(int argc, char **argv)
{
    if (logged_in_)
        return;

    LoginCommandParser login_command_parser;

    login_command_parser.ParseInput(argc, argv);

    if (login_command_parser.ShowHelpMessage())
        exit(0);

    login_command_parser.ValidateInput();

    port_ = login_command_parser.GetPort();
    hostname_ = login_command_parser.GetHostname();
    user_id_ = login_command_parser.GetUserid();

    set_device_id();

    login_server();

    // Inicializa o sync_dir
    if (local_directory_.empty()) {
        char *home_folder;

        if ((home_folder = getenv("HOME")) == nullptr) {
            home_folder = getpwuid(getuid())->pw_dir;
        }

        local_directory_ = StringFormatter() << home_folder << "/sync_dir_" << user_id_;
        boost::filesystem::create_directory(local_directory_);
    }

    load_info_from_disk();
}

void Client::set_device_id()
{
    // TODO Validar device_id encontrado no disco
    if (fs::exists(fs::path(device_id_file_))) {
        std::ifstream id_file(device_id_file_);
        std::getline(id_file, device_id_);
    } else {
        boost::uuids::uuid device_id = boost::uuids::random_generator()();
        device_id_ = to_string(device_id);

        std::ofstream id_file(device_id_file_);
        id_file << device_id_;
    }
}

void Client::login_server()
{
    if((socket_ = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        logger_->error("Error creating socket");
        throw std::runtime_error("Error trying to login to server");
    }

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    server_addr_.sin_addr.s_addr = inet_addr(hostname_.c_str());
    peer_length_ = sizeof(server_addr_);
    std::string command(StringFormatter() << "connect" << util::COMMAND_SEPARATOR_TOKEN
                                          << user_id_ << util::COMMAND_SEPARATOR_TOKEN << device_id_);

    send_command_and_expect_confirmation(command);

    char buffer[util::BUFFER_SIZE]{0};
    recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &server_addr_, &peer_length_);
    logger_->debug("Received from server {} port {} the message: {}",
                   inet_ntoa(server_addr_.sin_addr), ntohs(server_addr_.sin_port), buffer);
    server_addr_.sin_port = htons(static_cast<uint16_t>(std::stoi(buffer)));

    logged_in_ = true;
}

void Client::sync_client()
{
    // Recebe a lista de arquivos e remove o header
    auto server_files = list_server();
    server_files.erase(server_files.begin());

    // Atualiza as informações dos arquivos modificados e copia o buffer para permitir
    // que novas modificações possam ser adicionadas enquanto a sincronização acontece
    LockGuard modification_buffer_lock(modification_buffer_mutex_);
    for (auto& modified_file : modified_files_)
        update_modified_info(modified_file);
    auto modification_buffer = modified_files_;
    modification_buffer_lock.Unlock();

    // Compara as modificações locais com a lista de arquivos no servidor
    // Se as modificações locais são mais recentes envia ao servidor
    for (const auto& modified_file : modification_buffer) {
        auto server_file_iterator = std::find_if(server_files.begin(), server_files.end(),
                [&modified_file] (const std::vector<std::string>& server_file) ->
                        bool {return server_file[0] == modified_file.name;});

        // Se o arquivo foi deletado localmente deve ser deletado no servidor
        bool file_deleted = !fs::exists(fs::path(StringFormatter() << local_directory_ << "/" << modified_file.name));

        // Arquivo não existe no servidor, foi criado localmente e será enviado
        // Se o arquivo não existe localmente nada a fazer
        if (server_file_iterator == server_files.end()) {
            if (!file_deleted) {
                logger_->debug("Arquivo {} não existe no servidor, foi criado localmente e será enviado", modified_file.name);
                send_file(StringFormatter() << local_directory_ << "/" << modified_file.name);
            }
            continue;
        }

        // Modificação local é mais recente, envia ao servidor
        if (modified_file.last_modification_time > std::stoi(server_file_iterator->at(2))) {
            if (file_deleted) {
                logger_->debug("Modificação local é mais recente, envia ao servidor, chamando delete pra arquivo  {}",
                               modified_file.name);
                delete_file(modified_file.name);
            }
            else {
                logger_->debug("Modificação local é mais recente, envia ao servidor, chamando send_file pra arquivo  {}",
                               modified_file.name);
                send_file(StringFormatter() << local_directory_ << "/" << modified_file.name);
            }
            continue;
        }

        // Versão do servidor é mais recente, recebe
        if (modified_file.last_modification_time < std::stoi(server_file_iterator->at(2))) {
            logger_->debug("Versão do servidor é mais recente, recebe {}", modified_file.name);
            get_file(StringFormatter() << local_directory_ << "/" << modified_file.name);
        }
    }

    // Buffer para arquivos que foram removidos para atualizar o registro de arquivos
    std::vector<std::string> removed_files;

    // Copia a lista de arquivos do cliente para poder modificar os arquivos sem prejudicar
    // o algoritmo de iteração sobre a lista
    LockGuard user_files_lock(user_files_mutex_);
    auto user_files_buffer = user_files_;
    user_files_lock.Unlock();

    // Compara os arquivos locais com a lista de arquivos no servidor
    for (const auto& user_file : user_files_buffer) {
        auto modified_file_iterator = std::find_if(modification_buffer.begin(), modification_buffer.end(),
                                        [&user_file] (const util::file_info& modified_file) ->
                                                bool {return modified_file.name == user_file.name;});

        // Se o arquivo foi modificado já foi tratado
        if (modified_file_iterator != modification_buffer.end())
            continue;

        auto server_file_iterator = std::find_if(server_files.begin(), server_files.end(),
                                                 [&user_file] (const std::vector<std::string>& server_file) ->
                                                         bool {return server_file[0] == user_file.name;});

        // Se o arquivo não existe no servidor e não foi criado localmente o remove
        if (server_file_iterator == server_files.end()) {
            logger_->debug("Se o arquivo não existe no servidor e não foi criado localmente o remove {}", user_file.name);
            fs::remove(fs::path(StringFormatter() << local_directory_ << "/" << user_file.name));
            removed_files.emplace_back(user_file.name);
            continue;
        }

        // Verifica se o servidor está desatualizado, se estiver envia a versão mais recente
        if (user_file.last_modification_time > std::stoi(server_file_iterator->at(2))) {
            logger_->debug("Verifica se o servidor está desatualizado, se estiver envia a versão mais recente {}",
                           user_file.name);
            logger_->debug("Timestamp user_file {} server {}",
                           user_file.last_modification_time,
                           server_file_iterator->at(2));
            send_file(StringFormatter() << local_directory_ << "/" << user_file.name);
            continue;
        }

        // Versão do arquivo no servidor é mais recente, recebe
        if (user_file.last_modification_time < std::stoi(server_file_iterator->at(2))) {
            logger_->debug("Versão do arquivo no servidor é mais recente, recebe {}",
                           user_file.name);
            get_file(StringFormatter() << local_directory_ << "/" << user_file.name);
        }
    }

    // Remove os registros dos arquivos excluídos
    if (!removed_files.empty()) {
        user_files_lock.Lock();
        user_files_.erase(std::remove_if(user_files_.begin(), user_files_.end(),
                                         [&removed_files] (const dropbox_util::file_info &info) -> bool {
                                             return std::find(removed_files.begin(), removed_files.end(), info.name)
                                                    != removed_files.end();
                                         }),
                          user_files_.end());
        user_files_lock.Unlock();
    }

    // Verifica se existem arquivos novos no servidor, se existirem faz o download
    for (const auto& server_file : server_files) {
        auto modified_file_iterator = std::find_if(modification_buffer.begin(), modification_buffer.end(),
                                                   [&server_file] (const util::file_info& modified_file) ->
                                                           bool {return modified_file.name == server_file[0];});

        // Se o arquivo foi modificado já foi tratado
        if (modified_file_iterator != modification_buffer.end())
            continue;

        auto user_file_iterator = std::find_if(user_files_buffer.begin(), user_files_buffer.end(),
                                                   [&server_file] (const util::file_info& user_file) ->
                                                           bool {return user_file.name == server_file[0];});

        // Se o arquivo já existe no cliente já foi tratado
        if (user_file_iterator != user_files_buffer.end())
            continue;

        // Recebe o novo arquivo
        logger_->debug("Recebe o novo arquivo {}", server_file[0]);
        get_file(StringFormatter() << local_directory_ << "/" << server_file[0]);
    }

    // Limpa o buffer de arquivos modificados, removendo os que foram tratados durante a sincronização,
    // mas não removendo os que foram alterados novamente desde o início da sincronização
    modification_buffer_lock.Lock();
    for (const auto& modified_file : modification_buffer) {
        modified_files_.erase(std::remove_if(modified_files_.begin(), modified_files_.end(),
                                         [&] (const dropbox_util::file_info &info) -> bool {
                                             return info.name == modified_file.name &&
                                                     info.last_modification_time <= modified_file.last_modification_time;
                                         }),
                              modified_files_.end());
    }
    modification_buffer_lock.Unlock();
}

void Client::load_info_from_disk() {
    for (const fs::directory_entry& file : fs::directory_iterator(local_directory_)) {
        if (fs::is_directory(file.path())) {
            throw std::runtime_error("Unexpected subfolder on user folder");
        }

        std::string filename = file.path().string();
        std::string filename_without_path = file.path().filename().string();

        util::file_info local_file_info;
        local_file_info.name = filename_without_path;
        local_file_info.size = fs::file_size(filename);
        local_file_info.last_modification_time = fs::last_write_time(filename);

        user_files_.emplace_back(local_file_info);
    }
}

void Client::send_file(const std::string& complete_file_path)
{
    LockGuard lock(socket_mutex_);

    util::file_transfer_request request;
    request.in_file_path = complete_file_path;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    fs::path filepath(complete_file_path);
    std::string filename_without_path = filepath.filename().string();
    std::string local_file_path = StringFormatter() << local_directory_ << "/" << filename_without_path;

    std::string command(StringFormatter() << "upload" << util::COMMAND_SEPARATOR_TOKEN
                                          << filename_without_path << util::COMMAND_SEPARATOR_TOKEN << user_id_);

    send_command_and_expect_confirmation(command);
    util::File file_util;
    file_util.send_file(request);
}

void Client::get_file(const std::string& filename)
{
    LockGuard lock(socket_mutex_);

    util::file_transfer_request request;
    request.in_file_path = filename;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();
    std::string local_file_path = StringFormatter() << local_directory_ << "/" << filename_without_path;

    std::string command(StringFormatter() << "download" << util::COMMAND_SEPARATOR_TOKEN
                                          << filename_without_path << util::COMMAND_SEPARATOR_TOKEN << user_id_);

    send_command_and_expect_confirmation(command);

    util::File file_util;
    file_util.receive_file(request);

    lock.Unlock();

    // If the file was downloaded to the sync_dir folder update its info
    if (local_file_path == filename) {
        util::file_info received_file_info;
        received_file_info.name = filename_without_path;
        received_file_info.size = fs::file_size(local_file_path);
        received_file_info.last_modification_time = fs::last_write_time(local_file_path);

        LockGuard user_files_lock(user_files_mutex_);

        // Se já existe um registro do arquivo o remove para adição do novo registro
        util::remove_filename_from_list(filename_without_path, user_files_);

        user_files_.emplace_back(received_file_info);
    }
}

void Client::delete_file(const std::string& filename)
{
    LockGuard lock(socket_mutex_);

    std::string command(StringFormatter() << "remove" << util::COMMAND_SEPARATOR_TOKEN
                                          << filename << util::COMMAND_SEPARATOR_TOKEN << user_id_);
    send_command_and_expect_confirmation(command);
}

std::vector<std::vector<std::string>> Client::list_server()
{
    LockGuard lock(socket_mutex_);

    util::file_transfer_request request;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command(StringFormatter() << "list_server" << util::COMMAND_SEPARATOR_TOKEN << user_id_);
    logger_->debug("sent to server port {}" , ntohs(server_addr_.sin_port));

    send_command_and_expect_confirmation(command);

    util::File file_util;
    return file_util.receive_list_files(request);
}

std::vector<std::vector<std::string>> Client::list_client()
{
    std::vector<std::vector<std::string>> entries;
    std::vector<std::string> header {"name", "size", "modification_time"};
    entries.emplace_back(header);

    LockGuard user_files_lock(user_files_mutex_);

    for (const auto& file : user_files_) {
        std::vector<std::string> info;
        info.emplace_back(file.name);
        info.emplace_back(std::to_string(file.size));
        info.emplace_back(std::to_string(file.last_modification_time));
        entries.emplace_back(info);
    }

    return entries;
}

void Client::close_session()
{
    send_command_and_expect_confirmation("logout");
    logger_->debug("Client logged off");
    logged_in_ = false;
}

void Client::send_command_and_expect_confirmation(const std::string &command) {
    logger_->debug("Sent command {} to server. Expecting ACK", command);
    struct timeval set_timeout_val = {0, util::TIMEOUT_US},
                   unset_timeout_val = {0, 0};
    char ack[util::BUFFER_SIZE]{0};
    ssize_t received_bytes;

    setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &set_timeout_val, sizeof(set_timeout_val));
    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);
    received_bytes = recvfrom(socket_, ack, sizeof(ack), 0,(struct sockaddr *) &server_addr_, &peer_length_);
    setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &unset_timeout_val, sizeof(unset_timeout_val));

    if (received_bytes <= 0)
        throw std::runtime_error("Server is unreachable!");

    if (strcmp("ACK", ack) != 0) {
        std::string message = util::get_error_from_message(ack);

        throw std::logic_error(StringFormatter() << "Sent command " << command << " but failed to receive ACK. Received "
                                                                                    << message << " from server.");
    }

    logger_->debug("Received ACK from server");
}

void Client::update_modified_info(dropbox_util::file_info& info) {
    fs::path filepath {fs::path(StringFormatter() << local_directory_ << "/" << info.name)};

    std::time_t modification_time {fs::exists(filepath) ? fs::last_write_time(filepath) : info.last_modification_time};
    int64_t file_size {fs::exists(filepath) ? static_cast<int64_t>(fs::file_size(filepath)) : info.size};

    if (modification_time != info.last_modification_time || file_size != info.size) {
        // Se o buffer estava desatualizado a lista de arquivos do usuário também está, atualiza
        LockGuard user_files_lock(user_files_mutex_);

        auto user_file_iterator = std::find_if(user_files_.begin(), user_files_.end(),
                                               [&info] (const util::file_info& user_file) ->
                                                       bool {return user_file.name == info.name;});

        if (user_file_iterator != user_files_.end()) {
            user_file_iterator->size = file_size;
            user_file_iterator->last_modification_time = modification_time;
        }

        info.size = file_size;
        info.last_modification_time = modification_time;
    }
}
