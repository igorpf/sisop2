#include "../../util/include/dropboxUtil.hpp"
#include "../include/dropboxClient.hpp"

#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


void Client::login_server(const std::string& host, int port) {
    int peer_length;
    SOCKET sock;

    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cerr << "Error creating socket" << std::endl ;
        exit(DEFAULT_ERROR_CODE);
    }

    port_ = port;
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(port);
    server_addr_.sin_addr.s_addr = inet_addr(host.c_str());
    peer_length = sizeof(server_addr_);
    std::string command("connect ");
    command.append(user_id_)
            .append(" ")
            .append(std::to_string(device_id_));

    sendto(sock, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length);
    logged_in_ = true;
}


void Client::sync_client()
{
    throw std::logic_error("Function not implemented");
}

void Client::send_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void Client::get_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void Client::delete_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void Client::close_session()
{
    throw std::logic_error("Function not implemented");
}

Client::Client(uint64_t device_id_, const std::string &user_id_) : device_id_(device_id_), user_id_(user_id_) {}


