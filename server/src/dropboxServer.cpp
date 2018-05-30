#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxServer.hpp"
#include "../include/ClientThread.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include <boost/filesystem.hpp>

namespace util = dropbox_util;
namespace fs = boost::filesystem;

const std::string Server::LOGGER_NAME = "Server";

Server::Server() {
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
    logger_->set_level(spdlog::level::debug);
}

Server::~Server() {
    spdlog::drop(LOGGER_NAME);
}

void Server::start() {
    // Cria a pasta raíz do servidor na home do usuário
    char* home_folder;

    if ((home_folder = getenv("HOME")) == nullptr) {
        home_folder = getpwuid(getuid())->pw_dir;
    }

    local_directory_ = StringFormatter() << home_folder << "/dropbox_server";
    fs::create_directory(local_directory_);

    load_info_from_disk();

    // Inicializa o socket de comunicação
    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(util::get_errno_with_message("Error initializing socket"));

    memset((void *) &server_addr_, 0, sizeof(struct sockaddr_in));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(util::DEFAULT_SERVER_PORT));
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    peer_length_ = sizeof(server_addr_);
    port_ = util::DEFAULT_SERVER_PORT;

    if (bind(socket_, (struct sockaddr *) &server_addr_, peer_length_) == util::DEFAULT_ERROR_CODE)
        throw std::runtime_error(util::get_errno_with_message("Bind error"));
    logger_->info("Initialized socket of number {} for server", socket_);
    has_started_ = true;
}

void Server::load_info_from_disk() {
    for (const fs::directory_entry& entry : fs::directory_iterator(local_directory_)) {
        if (!fs::is_directory(entry.path())) {
            throw std::runtime_error("Unexpected file on server root folder");
        }

        std::string user_id = entry.path().filename().string();
        add_client(user_id, "");

        auto client_iterator = get_client_info(user_id);

        for (const fs::directory_entry& file : fs::directory_iterator(entry.path())) {
            if (fs::is_directory(file.path())) {
                throw std::runtime_error(StringFormatter() << "Unexpected subfolder on user " << user_id << " folder");
            }

            std::string user_file = file.path().string();
            std::string filename_without_path = file.path().filename().string();

            util::file_info user_file_info;
            user_file_info.name = filename_without_path;
            user_file_info.size = fs::file_size(user_file);
            user_file_info.last_modification_time = fs::last_write_time(user_file);

            client_iterator->user_files.emplace_back(user_file_info);
        }
    }
}

void Server::listen() {
    if (!has_started_)
        start();
    logger_->info("Server is listening on port {}", port_);
    char buffer[util::BUFFER_SIZE];
    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &current_client_, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(current_client_.sin_addr), ntohs(current_client_.sin_port), buffer);
            parse_command(buffer);
        } catch (std::exception& e) {
            logger_->error("Error parsing command from client {}", e.what());
            break;
        }
    }
}

void Server::receive_file(const std::string& filename, const std::string &user_id) {
    util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = server_addr_;
    request.peer_length = peer_length_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();
    std::string local_file_path = StringFormatter()
            << local_directory_ << "/" << user_id << "/" << filename_without_path;

    request.in_file_path = local_file_path;

    util::File file_util;
    file_util.receive_file(request);

    util::file_info received_file_info;
    received_file_info.name = filename_without_path;
    received_file_info.size = fs::file_size(local_file_path);
    received_file_info.last_modification_time = fs::last_write_time(local_file_path);

    auto client_iterator = get_client_info(user_id);

    // Se já existe um registro do arquivo o remove para adição do novo registro
    remove_file_from_client(user_id, filename);

    client_iterator->user_files.emplace_back(received_file_info);
}

void Server::send_file(const std::string& filename, const std::string &user_id) {
    util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = current_client_;
    request.peer_length = peer_length_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    request.in_file_path = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;

    util::File file_util;
    file_util.send_file(request);
}

void Server::delete_file(const std::string& filename, const std::string &user_id) {
    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    std::string server_file = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;
    fs::remove(server_file);

    remove_file_from_client(user_id, filename_without_path);
}

void Server::list_server(const std::string &user_id) {
    auto client_iterator = get_client_info(user_id);
    std::string user_file_list {"name;size;modification_time&"};

    for (const auto& file : client_iterator->user_files) {
        user_file_list.append(StringFormatter() << file.name << ';'<< file.size << ';'
                                                << file.last_modification_time << '&');
    }

    // Remove último &
    user_file_list.pop_back();

    util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = current_client_;
    request.peer_length = peer_length_;

    util::File file_util;
    file_util.send_list_files(request, user_file_list);
}

void Server::sync_server() {
    throw std::logic_error("Function not implemented");
}

void Server::parse_command(const std::string &command_line) {
    // TODO Validar parâmetros
    logger_->debug("Parsing command {}", command_line);
    auto tokens = util::split_words_by_token(command_line);
    auto command = tokens[0];
    if (command == "connect") {
        auto user_id = tokens[1], device_id = tokens[2];
        add_client(user_id, device_id);
    } else if (command == "download") {
        send_file(tokens[1], tokens[2]);
    } else if (command == "upload") {
        receive_file(tokens[1], tokens[2]);
    } else if (command == "remove") {
        delete_file(tokens[1], tokens[2]);
    } else if (command == "list_server") {
        list_server(tokens[1]);
    }
    // TODO Erro ao receber comando invãlido
}

void Server::add_client(const std::string& user_id, const std::string& device_id) {
    if (!has_client_connected(user_id)) {
        client_info new_client;
        new_client.user_id = user_id;
        if (!device_id.empty())
            new_client.user_devices.emplace_back(device_id);
        clients_.push_back(new_client);

        std::string client_path = StringFormatter() << local_directory_ << "/" << user_id;
        fs::create_directory(client_path);
        thread_pool_.add_client("ClientThread" + user_id + device_id, inet_ntoa(current_client_.sin_addr), ntohs(current_client_.sin_port));

        logger_->info("Connected new client, total clients: {}",  clients_.size());
    }

    auto client_iterator = get_client_info(user_id);

    // Verifica se o dispositivo não está na lista
    if (std::find(client_iterator->user_devices.begin(), client_iterator->user_devices.end(), device_id)
        == client_iterator->user_devices.end())
        // Se não estiver, verifica se o cliente ainda pode adicionar dispositivos
        if (client_iterator->user_devices.size() < MAX_CLIENT_DEVICES)
            client_iterator->user_devices.emplace_back(device_id);
        // TODO Adicionar mensagem de erro para demais dispositivos
        // TODO Ignorar/responder com erro requests de demais dispositivos
}

bool Server::has_client_connected(const std::string &client_id) {
    auto client_iterator = get_client_info(client_id);
    return client_iterator != clients_.end();
}

std::vector<client_info>::iterator Server::get_client_info(const std::string& user_id) {
    return std::find_if(clients_.begin(), clients_.end(),
                        [&user_id] (const client_info& c) -> bool {return user_id == c.user_id;});
}

void Server::remove_file_from_client(const std::string &user_id, const std::string &filename) {
    auto client_iterator = get_client_info(user_id);
    if (!client_iterator->user_files.empty())
        client_iterator->user_files.erase(std::remove_if(client_iterator->user_files.begin(),
                                                         client_iterator->user_files.end(),
                                                         [&filename] (const dropbox_util::file_info& info) ->
                                                                 bool {return filename == info.name;}),
                                          client_iterator->user_files.end());
}
