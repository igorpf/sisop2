#include "../../util/include/dropboxUtil.hpp"
#include "../include/dropboxServer.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Utility functions
std::vector<std::string> split_tokens(const std::string &str) {
    std::stringstream stream(str);
    std::string buffer;
    std::vector<std::string> tokens;
    while(stream >> buffer)
        tokens.push_back(buffer);
    return tokens;
}

std::string get_errno_with_message(const std::string &base_message = "") {
    std::ostringstream str_stream;
    str_stream << base_message << ", error code " << errno << std::endl;
    return str_stream.str();
}


// Server methods
bool Server::has_client_connected(const std::string &client_id) {
    auto client_iterator = std::find_if(clients_.begin(), clients_.end(),
                                     [&](const client& c) -> bool { return client_id == c.user_id;});
    return !(client_iterator == clients_.end());
}

void Server::parse_command(const std::string &command_line) {
    auto tokens = split_tokens(command_line);
    auto command = tokens[0];
    if(command == "connect") {
        auto user_id = tokens[1], device_id = tokens[2];
        add_client(user_id, static_cast<uint64_t>(std::stoi(device_id)));
    } else if (command == "download") {
        send_file(tokens[1]);
    } else if (command == "upload") {
        receive_file(tokens[1]);
    }
}


void Server::add_client(const std::string &user_id, uint64_t device_id) {
    if(!has_client_connected(user_id)) {
        client new_client;
        new_client.user_id = user_id;
        new_client.logged_in;
        clients_.push_back(new_client);
        logger_->info( "Connected new client, total clients: {}",  clients_.size());
    }
    auto client_iterator = std::find_if(clients_.begin(), clients_.end(),
                                     [&](const client& c) -> bool { return user_id == c.user_id;});
    client_iterator->devices.insert(device_id);
}

void Server::sync_server()
{
    throw std::logic_error("Function not implemented");
}

void Server::receive_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void Server::send_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void start_server()
{
    file_transfer_request request;
    request.ip = std::string(LOOPBACK_IP);
    request.port = DEFAULT_SERVER_PORT;
    DropboxUtil::File file_util;
    file_util.receive_file(request);
}

void Server::start(int32_t port) {
    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(get_errno_with_message("Error initializing socket"));

    memset((void *) &server_addr_, 0, sizeof(struct sockaddr_in));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port));
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    peer_length_ = sizeof(server_addr_);
    port_ = port;

    if(bind(socket_, (struct sockaddr *) &server_addr_, static_cast<socklen_t>(peer_length_)) == DEFAULT_ERROR_CODE)
        throw std::runtime_error(get_errno_with_message("Bind error"));
    logger_->info("Initialized socket of number {} for server", socket_);
    has_started_ = true;
}

void Server::listen() {
    if(!has_started_)
        throw std::logic_error("The server must be initialized to begin listening");
    logger_->info("Server is listening on port {}", port_);
    bool continue_listening;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client{};
    ssize_t received_bytes;
    do {
        received_bytes = recvfrom(socket_, buffer, sizeof(buffer), 0,(struct sockaddr *) &client, (socklen_t *)&peer_length_);
        logger_->debug("Received from client {} port {} the message: {}", inet_ntoa(client.sin_addr), client.sin_port, buffer);
        continue_listening = !(received_bytes == 1 && buffer[0] == -1);
        parse_command(buffer);
    } while(continue_listening);
}

Server::Server() {
    logger_ = spdlog::stdout_color_mt("Server");
    logger_->set_level(spdlog::level::debug);
}

