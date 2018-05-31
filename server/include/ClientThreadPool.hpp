#ifndef DROPBOX_CLIENTTHREADPOOL_HPP
#define DROPBOX_CLIENTTHREADPOOL_HPP

#include <vector>
#include <memory>
#include "ClientThread.hpp"

class ClientThreadPool {
public:
    void add_client(const std::string &logger_name, const std::string &ip, int32_t port, dropbox_util::SOCKET socket);
private:
    std::vector<std::shared_ptr<ClientThread>> threads_;
};
#endif //DROPBOX_CLIENTTHREADPOOL_HPP
