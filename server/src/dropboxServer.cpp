#include "../../util/include/dropboxUtil.hpp"
#include "../include/dropboxServer.hpp"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <iostream>

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
    auto cli_iterator = std::find_if(clients_.begin(), clients_.end(),
                                     [&](const client& c) -> bool { return client_id == c.user_id;});
    return !(cli_iterator == clients_.end());
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
        std::cout << "Connected new client, total clients: " << clients_.size() << std::endl;
    }
    auto cli_iterator = std::find_if(clients_.begin(), clients_.end(),
                                     [&](const client& c) -> bool { return user_id == c.user_id;});
    cli_iterator->devices.insert(device_id);
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


const int port = 9001;

void start_server()
{
    file_transfer_request request;
    request.ip = std::string("127.0.0.1");
    request.port = port;
    receive_file(request);
}

void Server::start(uint16_t port) {
    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(get_errno_with_message("Error initializing socket"));

    memset((void *) &server_addr_, 0, sizeof(struct sockaddr_in));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(port);
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    peer_length_ = sizeof(server_addr_);

    if(bind(socket_, (struct sockaddr *) &server_addr_, peer_length_) == DEFAULT_ERROR_CODE)
        throw std::runtime_error(get_errno_with_message("Bind error"));
    std::cout << "Initialized socket of number " << socket_ << " for server" << std::endl;
    has_started_ = true;
}

void Server::listen() {
    if(!has_started_)
        throw std::logic_error("The server must be initialized to begin listening");
    std::cout << "Server is listening on port " << port_ << std::endl;
    bool continue_listening;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client;
    ssize_t received_bytes;
    do {
        received_bytes = recvfrom(socket_, buffer, sizeof(buffer), 0,(struct sockaddr *) &client, (socklen_t *)&peer_length_);
        std::cout << "Received from client " << (inet_ntoa(client.sin_addr))
                  << " port " << client.sin_port
                  << " the message: " << buffer << std::endl;
        continue_listening = !(received_bytes == 1 && buffer[0] == -1);
        parse_command(buffer);
    } while(continue_listening);
}

