#include "../include/dropboxServer.hpp"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


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

std::string get_errno_with_message(const std::string &base_message = "") {
    std::ostringstream str_stream;
    str_stream << base_message << ", error code " << errno << std::endl;
    return str_stream.str();
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
    } while(continue_listening);
}