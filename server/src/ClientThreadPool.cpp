#include <memory>
#include <arpa/inet.h>
#include "../include/ClientThreadPool.hpp"

void ClientThreadPool::add_client(dropbox_util::new_client_param_list client_param_list) {
    auto client = std::make_shared<ClientThread>(local_directory_, client_param_list.logger_name, client_param_list.ip,
                                                 client_param_list.port, client_param_list.socket, client_param_list.info);
    threads_.push_back(client);
    client->Start();
}

void ClientThreadPool::set_local_directory(const std::string &local_directory) {
    ClientThreadPool::local_directory_ = local_directory;
}
