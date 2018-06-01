#ifndef DROPBOX_CLIENTTHREADPOOL_HPP
#define DROPBOX_CLIENTTHREADPOOL_HPP

#include <vector>
#include <memory>
#include "../../util/include/dropboxUtil.hpp"
#include "ClientThread.hpp"

class ClientThreadPool {
public:
    void add_client(const std::string &logger_name, const std::string &ip, int32_t port, dropbox_util::SOCKET socket);

    void set_local_directory(const std::string &local_directory);

private:
    std::vector<std::shared_ptr<ClientThread>> threads_;
    std::string local_directory_;
};
#endif //DROPBOX_CLIENTTHREADPOOL_HPP
