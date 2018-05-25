#include "../../util/include/dropboxUtil.hpp"
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

const std::string Server::LOGGER_NAME = "Server";

Server::Server() {
    logger_ = LoggerFactory::getLoggerForName(LOGGER_NAME);
}

Server::~Server() {
    spdlog::drop(LOGGER_NAME);
}

bool Server::has_client_connected(const std::string &client_id) {
    auto client_iterator = std::find_if(clients_.begin(), clients_.end(),
                                     [&](const client& c) -> bool { return client_id == c.user_id;});
    return client_iterator != clients_.end();
}

void Server::parse_command(struct sockaddr_in &client, const std::string &command_line) {
    logger_->debug("Parsing command {}", command_line);
    auto tokens = util::split_words_by_token(command_line);
    auto command = tokens[0];
    if (command == "connect") {
        auto user_id = tokens[1], device_id = tokens[2];
        add_client(user_id, device_id);
        send_command_confirmation(client);
    } else if (command == "download") {
        send_command_confirmation(client);
        send_file(tokens[1]);
    } else if (command == "upload") {
        send_command_confirmation(client);
        receive_file(tokens[1]);
    }
}

void Server::add_client(const std::string &user_id, const std::string& device_id) {
    if(!has_client_connected(user_id)) {
        client new_client;
        new_client.user_id = user_id;
        new_client.logged_in = true;
        clients_.push_back(new_client);
        logger_->info( "Connected new client, total clients: {}",  clients_.size());
    }
    auto client_iterator = std::find_if(clients_.begin(), clients_.end(),
                                     [&](const client& c) -> bool { return user_id == c.user_id;});
    client_iterator->devices.push_back(device_id);
}

void Server::receive_file(const std::string& filename) {
    util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = server_addr_;
    request.peer_length = peer_length_;
    request.in_file_path = filename;

    util::File file_util;
    file_util.receive_file(request);
}

void Server::start(int32_t port) {
    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(util::get_errno_with_message("Error initializing socket"));

    memset((void *) &server_addr_, 0, sizeof(struct sockaddr_in));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port));
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    peer_length_ = sizeof(server_addr_);
    port_ = port;

    if(bind(socket_, (struct sockaddr *) &server_addr_, peer_length_) == util::DEFAULT_ERROR_CODE)
        throw std::runtime_error(util::get_errno_with_message("Bind error"));
    logger_->info("Initialized socket of number {} for server", socket_);
    has_started_ = true;
}

void Server::listen() {
    if(!has_started_)
        start();
    logger_->info("Server is listening on port {}", port_);
    char buffer[util::BUFFER_SIZE];
    struct sockaddr_in client{};
    while(true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buffer);
            parse_command(client, buffer);
        } catch (std::string &e) {
            logger_->error("Error parsing command from client {}", e);
            send_command_error_message(client, e);
        } catch (std::exception &e) {
            logger_->error("Fatal error parsing command from client {}. Stopping transmission", e.what());
            break;
        }
    }
}

void Server::sync_server() {
    throw std::logic_error("Function not implemented");
}

void Server::send_file(const std::string& filename) {
    throw std::logic_error("Function not implemented");
}

void Server::send_command_confirmation(struct sockaddr_in &client) {
    sendto(socket_, "ACK", 4, 0, (struct sockaddr *)&client, peer_length_);
}

void Server::send_command_error_message(struct sockaddr_in &client, const std::string &error_message)  {
    sendto(socket_, error_message.c_str(), error_message.size(), 0, (struct sockaddr *)&client, peer_length_);
}

