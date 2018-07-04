#ifndef DROPBOX_CLIENTTHREADPOOL_HPP
#define DROPBOX_CLIENTTHREADPOOL_HPP

#include <vector>
#include <memory>

#include "../../util/include/dropboxUtil.hpp"

#include "ClientThread.hpp"
#include "backup_file_sync_thread.hpp"

class ClientThreadPool {
public:
    virtual ~ClientThreadPool();

    void add_client(dropbox_util::new_client_param_list client_param_list);

    void add_backup_server(const std::string& logger_name, const std::string& ip,
                           int64_t port, dropbox_util::SOCKET socket);

    void set_local_directory(const std::string &local_directory);

    static void remove_client_thread_wrapper(ClientThreadPool &pool, const std::string &user_id,
                                             const std::string &device_id);

    /**
     * device_id is a string copy on purpose because it's not available on the caller scope anymore
     */
    void remove_client_thread(const std::string &user_id, std::string device_id);

    void setDisconnectClientCallback(std::function<void(std::string, std::string)> &disconnect_client_callback);

private:
    std::function<void(std::string, std::string)> disconnect_client_callback_;
    std::vector<std::shared_ptr<ClientThread>> threads_;
    std::map<std::string, pthread_mutex_t> locks_;

    std::vector<std::shared_ptr<BackupFileSyncThread>> backup_threads_;

    std::string local_directory_;
};

#endif //DROPBOX_CLIENTTHREADPOOL_HPP
