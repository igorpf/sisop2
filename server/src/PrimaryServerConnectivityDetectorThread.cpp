#include <arpa/inet.h>
#include "../include/PrimaryServerConnectivityDetectorThread.hpp"
#include "../../util/include/string_formatter.hpp"

static const std::string LOGGER_NAME = "PrimaryServerConnectivityDetectorThread";

PrimaryServerConnectivityDetectorThread::PrimaryServerConnectivityDetectorThread() : logger_(LOGGER_NAME) {}

void PrimaryServerConnectivityDetectorThread::Run() {
    init_server_address();

    logger_->info("Started thread. Primary server is on ip {} port {}", primary_server_ip_, primary_server_port_);
    while(is_backup_) {
        std::string command = StringFormatter() << dropbox_util::CHECK_PRIMARY_SERVER_MESSAGE;
        char ack[dropbox_util::BUFFER_SIZE]{0};
        ssize_t received_bytes;
        int8_t retries = 0;
        do {
            sendto(socket_, command.c_str() , command.size(), 0, (struct sockaddr *)&primary_server_addr, sizeof(primary_server_addr));
            received_bytes = recvfrom(socket_, ack, sizeof(ack), 0,(struct sockaddr *) &primary_server_addr, &peer_length_);
            logger_->debug("Received {} from server", ack);
            retries++;
        } while (received_bytes <= 0 && retries < MAX_RETRIES);

        if(retries == MAX_RETRIES) {
            logger_->error("Primary server has been disconnected, notifying server");
            notify_primary_server_disconnection_callback__();
        }
    }
    logger_->info("This server has become the primary server, terminating thread.");
}

void PrimaryServerConnectivityDetectorThread::init_server_address() {
    if((socket_ = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        logger_->error("Error creating socket");
        throw std::runtime_error("Error trying to create socket");
    }
    primary_server_addr.sin_family = AF_INET;
    primary_server_addr.sin_port = htons(static_cast<uint16_t>(primary_server_port_));
    primary_server_addr.sin_addr.s_addr = inet_addr(primary_server_ip_.c_str());
    peer_length_ = sizeof(primary_server_addr);
    struct timeval set_timeout_val = {0, dropbox_util::TIMEOUT_US};
    setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &set_timeout_val, sizeof(set_timeout_val));
}

void PrimaryServerConnectivityDetectorThread::setNotifyCallback(
        const std::function<void()> &notify_primary_server_disconnection_callback__) {
    PrimaryServerConnectivityDetectorThread::notify_primary_server_disconnection_callback__ = notify_primary_server_disconnection_callback__;
}

void PrimaryServerConnectivityDetectorThread::setPrimaryServerIp(const std::string &primary_server_ip_) {
    PrimaryServerConnectivityDetectorThread::primary_server_ip_ = primary_server_ip_;
}

void PrimaryServerConnectivityDetectorThread::setPrimaryServerPort(int64_t primary_server_port_) {
    PrimaryServerConnectivityDetectorThread::primary_server_port_ = primary_server_port_;
}

void PrimaryServerConnectivityDetectorThread::stop() {
    is_backup_ = false;
    Join();
}
