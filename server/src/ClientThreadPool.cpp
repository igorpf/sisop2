#include <memory>
#include <arpa/inet.h>
#include "../include/ClientThreadPool.hpp"

void ClientThreadPool::add_client(const std::string &logger_name, const std::string &ip, int32_t port) {
    auto client = std::make_shared<ClientThread>(logger_name, ip, port);

    threads_.push_back(client);
    client->Start();
}
