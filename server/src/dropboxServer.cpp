#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxServer.hpp"

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
    boost::filesystem::create_directory(local_directory_);

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

    if(bind(socket_, (struct sockaddr *) &server_addr_, peer_length_) == util::DEFAULT_ERROR_CODE)
        throw std::runtime_error(util::get_errno_with_message("Bind error"));
    logger_->info("Initialized socket of number {} for server", socket_);
    has_started_ = true;
}

void Server::load_info_from_disk() {
    // TODO Carregar as informações já disponíveis em disco (usernames e file_infos)
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
                           inet_ntoa(current_client_.sin_addr), current_client_.sin_port, buffer);
            parse_command(buffer);
        } catch (std::exception &e) {
            logger_->error("Error parsing command from client {}", e.what());
        }
    }
}

void Server::receive_file(const std::string& filename, const std::string &user_id) {
    util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = server_addr_;
    request.peer_length = peer_length_;

    boost::filesystem::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();
    std::string local_file_path = StringFormatter()
            << local_directory_ << "/"<< user_id << "/" << filename_without_path;

    request.in_file_path = local_file_path;

    util::File file_util;
    file_util.receive_file(request);

    util::file_info received_file_info;
    received_file_info.name = filename_without_path;
    received_file_info.size = boost::filesystem::file_size(local_file_path);
    received_file_info.last_modification_time = boost::filesystem::last_write_time(local_file_path);

    auto client_iterator = get_client_info(user_id);
    client_iterator->user_files.emplace_back(received_file_info);
}

void Server::send_file(const std::string& filename, const std::string &user_id) {
    util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = current_client_;
    request.peer_length = peer_length_;

    boost::filesystem::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    request.in_file_path = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;

    util::File file_util;
    file_util.send_file(request);
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
    logger_->debug("Parsing command {}", command_line);
    auto tokens = util::split_words_by_spaces(command_line);
    auto command = tokens[0];
    if (command == "connect") {
        auto user_id = tokens[1], device_id = tokens[2];
        add_client(user_id, device_id);
    } else if (command == "download") {
        send_file(tokens[1], tokens[2]);
    } else if (command == "upload") {
        receive_file(tokens[1], tokens[2]);
    } else if (command == "list_server") {
        list_server(tokens[1]);
    }
}

void Server::add_client(const std::string &user_id, const std::string& device_id) {
    if (!has_client_connected(user_id)) {
        client_info new_client;
        new_client.logged_in = true;
        new_client.user_id = user_id;
        clients_.push_back(new_client);

        std::string client_path = StringFormatter() << local_directory_ << "/" << user_id;
        boost::filesystem::create_directory(client_path);

        logger_->info("Connected new client, total clients: {}",  clients_.size());
    }

    auto client_iterator = get_client_info(user_id);
    client_iterator->user_devices.push_back(device_id);
    client_iterator->logged_in = true;
}

bool Server::has_client_connected(const std::string &client_id) {
    auto client_iterator = get_client_info(client_id);
    return client_iterator != clients_.end();
}

std::vector<client_info>::iterator Server::get_client_info(const std::string& user_id) {
    return std::find_if(clients_.begin(), clients_.end(),
                        [&user_id](const client_info& c) -> bool {return user_id == c.user_id;});
}
