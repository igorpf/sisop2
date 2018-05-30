#include <iostream>
#include <arpa/inet.h>
#include "../include/ClientThread.hpp"
#include "../../util/include/dropboxUtil.hpp"

ClientThread::ClientThread(const std::string &logger_name, const std::string &ip, int32_t port) : logger_name_(
        logger_name), ip_(ip), port_(port) {
    logger_ = spdlog::stdout_color_mt(logger_name);
}

ClientThread::~ClientThread() {
    spdlog::drop(logger_name_);
}

void ClientThread::Run() {
    start_socket();
    char buffer[dropbox_util::BUFFER_SIZE];
    logger_->info("Starting to listen:");
    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr_, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(client_addr_.sin_addr), client_addr_.sin_port, buffer);

        } catch (std::exception& e) {
            logger_->error("Error parsing command from client {}", e.what());
            break;
        }
    }
}

void ClientThread::start_socket() {
    logger_->info("New client thread started, ip {}, port {}", ip_, port_);
    client_addr_.sin_family = AF_INET;
    client_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    client_addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    peer_length_ = sizeof(client_addr_);
    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(dropbox_util::get_errno_with_message("Error initializing socket"));
    if (bind(socket_, (struct sockaddr *) &client_addr_, peer_length_) == dropbox_util::DEFAULT_ERROR_CODE)
        throw std::runtime_error(dropbox_util::get_errno_with_message("Bind error"));
    logger_->info("Initialized socket {} from client {} port {}", socket_,
                  inet_ntoa(client_addr_.sin_addr), client_addr_.sin_port);

    // TODO Remove this
    sendto(socket_, "helo boi", 9, 0, (struct sockaddr*)&client_addr_, peer_length_);
}
