#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/File.hpp"
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

const std::string Client::LOGGER_NAME = "Client";

Client::Client(uint64_t device_id, const std::string &user_id) : device_id_(device_id), user_id_(user_id) {
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
    logger_->set_level(spdlog::level::debug);
}

Client::~Client() {
    spdlog::drop(LOGGER_NAME);
}

void Client::login_server(const std::string& host, int32_t port) {
    if((socket_ = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        logger_->error("Error creating socket");
        throw std::runtime_error("Error trying to login to server");
    }

    port_ = port;
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port));
    server_addr_.sin_addr.s_addr = inet_addr(host.c_str());
    peer_length_ = sizeof(server_addr_);
    std::string command("connect ");
    command.append(user_id_)
            .append(" ")
            .append(std::to_string(device_id_));

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr*)&server_addr_, peer_length_);
    logged_in_ = true;
}

void Client::send_file(const std::string& filename)
{
    util::file_transfer_request request;
    request.in_file_path = filename;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command("upload ");
    command.append(filename);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);
    util::File file_util;
    file_util.send_file(request);
}

void Client::close_session() {
    throw std::logic_error("Function not implemented");
}

void Client::sync_client() {
    throw std::logic_error("Function not implemented");
}

void Client::get_file(const std::string& filename) {
    throw std::logic_error("Function not implemented");
}

void Client::delete_file(const std::string& filename) {
    throw std::logic_error("Function not implemented");
}