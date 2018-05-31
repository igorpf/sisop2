#include <iostream>
#include <arpa/inet.h>
#include "../include/ClientThread.hpp"
#include "../../util/include/dropboxUtil.hpp"

ClientThread::ClientThread(const std::string &logger_name, const std::string &ip, int32_t port,
                           dropbox_util::SOCKET socket) : logger_name_(logger_name), ip_(ip), port_(port),
                                                           socket_(socket) {
    logger_ = spdlog::stdout_color_mt(logger_name);
    logger_->set_level(spdlog::level::debug);
}

ClientThread::~ClientThread() {
    spdlog::drop(logger_name_);
}

void ClientThread::Run() {
    logger_->info("New client thread started, socket {} ip {}, port {}", socket_, ip_, port_);
    char buffer[dropbox_util::BUFFER_SIZE];

    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            logger_->debug("Waiting message from client {} port {}",
                           inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port));
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr_, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port), buffer);

        } catch (std::exception& e) {
            logger_->error("Error parsing command from client {}", e.what());
            break;
        }
    }
}