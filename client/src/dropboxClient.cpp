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

#include "../../util/include/string_formatter.hpp"
#include "../../util/include/table_printer.hpp"
#include "../../util/include/File.hpp"
#include "../../util/include/lock_guard.hpp"
#include "../include/login_command_parser.hpp"

#include "../include/dropboxClient.hpp"

namespace fs = boost::filesystem;
namespace util = dropbox_util;

const std::string Client::LOGGER_NAME = "Client";

Client::Client()
{
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
    logger_->set_level(spdlog::level::debug);
}

Client::~Client()
{
    spdlog::drop(LOGGER_NAME);
}

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

    boost::uuids::name_generator_sha1 id_generator(boost::uuids::ns::dns());
    boost::uuids::uuid device_id = id_generator(user_id_);
    device_id_ = to_string(device_id);

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

void Client::login_server()
{
    // TODO Shouldn't login fail if the server doesn't answer?
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

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr*)&server_addr_, peer_length_);
    logged_in_ = true;
}

void Client::sync_client()
{
    // Recebe a lista de arquivos e remove o header
    auto server_files = list_server();
    server_files.erase(server_files.begin());

    // Copia o buffer de arquivos modificados para permitir que novas modificações
    // possam ser adicionadas enquanto a sincronização acontece
    LockGuard modification_buffer_lock(modification_buffer_mutex_);
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
            if (!file_deleted)
                send_file(StringFormatter() << local_directory_ << "/" << modified_file.name);
            continue;
        }

        // Modificação local é mais recente, envia ao servidor
        if (std::to_string(modified_file.last_modification_time) > server_file_iterator->at(2)) {
            if (file_deleted)
                delete_file(modified_file.name);
            else
                send_file(StringFormatter() << local_directory_ << "/" << modified_file.name);
            continue;
        }

        // Versão do servidor é mais recente, recebe
        if (std::to_string(modified_file.last_modification_time) < server_file_iterator->at(2))
            get_file(StringFormatter() << local_directory_ << "/" << modified_file.name);
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
            fs::remove(fs::path(StringFormatter() << local_directory_ << "/" << user_file.name));
            removed_files.emplace_back(user_file.name);
            continue;
        }

        // Verifica se o servidor está desatualizado, se estiver envia a versão mais recente
        if (std::to_string(user_file.last_modification_time) > server_file_iterator->at(2)) {
            send_file(StringFormatter() << local_directory_ << "/" << user_file.name);
            continue;
        }

        // Versão do arquivo no servidor é mais recente, recebe
        if (std::to_string(user_file.last_modification_time) < server_file_iterator->at(2))
            get_file(StringFormatter() << local_directory_ << "/" << user_file.name);
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
        get_file(StringFormatter() << local_directory_ << "/" << server_file[0]);
    }

    // Limpa o buffer de arquivos modificados, removendo os que foram tratados durante a sincronização,
    // mas não removendo os que foram alterados novamente desde o início da sincronização
    modification_buffer_lock.Lock();
    for (const auto& modified_file : modification_buffer) {
        modified_files_.erase(std::remove_if(modified_files_.begin(), modified_files_.end(),
                                         [&modified_file] (const dropbox_util::file_info &info) -> bool {
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

void Client::send_file(const std::string& filename)
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

    std::string command(StringFormatter() << "upload" << util::COMMAND_SEPARATOR_TOKEN
                                          << filename_without_path << util::COMMAND_SEPARATOR_TOKEN << user_id_);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);

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

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);

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
        if (!user_files_.empty())
            user_files_.erase(std::remove_if(user_files_.begin(), user_files_.end(),
                                             [&filename_without_path] (const dropbox_util::file_info& info) ->
                                                     bool {return filename_without_path == info.name;}),
                              user_files_.end());

        user_files_.emplace_back(received_file_info);
    }
}

void Client::delete_file(const std::string& filename)
{
    LockGuard lock(socket_mutex_);

    std::string command(StringFormatter() << "remove" << util::COMMAND_SEPARATOR_TOKEN
                                          << filename << util::COMMAND_SEPARATOR_TOKEN << user_id_);
    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);
}

std::vector<std::vector<std::string>> Client::list_server()
{
    LockGuard lock(socket_mutex_);

    util::file_transfer_request request;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command(StringFormatter() << "list_server" << util::COMMAND_SEPARATOR_TOKEN << user_id_);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);

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
    // TODO(jfguimaraes) Implement client logoff
    logger_->debug("Client logged off");
    logged_in_ = false;
}
