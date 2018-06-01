#include <memory>
#include <arpa/inet.h>
#include "../include/ClientThreadPool.hpp"

void ClientThreadPool::add_client(const std::string &logger_name, const std::string &ip, int32_t port, dropbox_util::SOCKET socket) {
    auto client = std::make_shared<ClientThread>(local_directory_, logger_name, ip, port, socket);
    threads_.push_back(client);
    client->Start();
}

void ClientThreadPool::set_local_directory(const std::string &local_directory) {
    ClientThreadPool::local_directory_ = local_directory;
}
