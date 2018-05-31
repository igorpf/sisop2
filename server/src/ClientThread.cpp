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
    init_client_address();
    char buffer[dropbox_util::BUFFER_SIZE];
    logger_->info("Starting to listen:");
    struct sockaddr_in client_addr{0};
    socklen_t peer_length_;
    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            logger_->debug("Waiting message from client {} port {}",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        } catch (std::exception& e) {
            logger_->error("Error parsing command from client {}", e.what());
            break;
        }
    }
}

void ClientThread::init_client_address() {
    logger_->info("New client thread started, ip {}, port {}", ip_, port_);

    client_addr_.sin_family = AF_INET;
    client_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    client_addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    peer_length_ = sizeof(client_addr_);

    logger_->info("Initialized socket {} from client {} port {}", socket_,
                  inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port));

}
