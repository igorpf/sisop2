#ifndef DROPBOX_CLIENTTHREADPOOL_HPP
#define DROPBOX_CLIENTTHREADPOOL_HPP

#include <vector>
#include <memory>
#include "../../util/include/dropboxUtil.hpp"
#include "ClientThread.hpp"

class ClientThreadPool {
public:
    virtual ~ClientThreadPool();

    void add_client(dropbox_util::new_client_param_list client_param_list);

    void set_local_directory(const std::string &local_directory);

    static void remove_client_thread(ClientThreadPool &pool, const std::string &user_id, const std::string &device_id);

    std::vector<std::shared_ptr<ClientThread>> &getThreads();

private:
    std::vector<std::shared_ptr<ClientThread>> threads_;
    std::map<std::string, pthread_mutex_t> locks_;

    std::string local_directory_;
};
#endif //DROPBOX_CLIENTTHREADPOOL_HPP
