#include "../include/ClientThreadPool.hpp"

#include <memory>
#include <arpa/inet.h>



void ClientThreadPool::add_client(dropbox_util::new_client_param_list client_param_list) {
    auto client_lock_iterator = locks_.find(client_param_list.user_id);
    if (client_lock_iterator == locks_.end()) {
        locks_[client_param_list.user_id] = PTHREAD_MUTEX_INITIALIZER;
    }

    client_thread_param_list param_list
            {
                client_param_list.user_id, client_param_list.device_id, local_directory_,
                client_param_list.logger_name, client_param_list.ip,
                client_param_list.port, client_param_list.socket, client_param_list.info,
                locks_.find(client_param_list.user_id)->second,
                client_param_list.frontend_port,
                client_param_list.clients_buffer_mutex, client_param_list.clients_buffer
            };

    auto client = std::make_shared<ClientThread>(param_list);
    threads_.push_back(client);
    client->setLogoutCallback(std::bind(remove_client_thread_wrapper, std::ref(*this), client_param_list.user_id,
                                        client_param_list.device_id));
    client->Start();
}

void ClientThreadPool::add_backup_server(const std::string& logger_name, const std::string& ip,
                                         int64_t port, dropbox_util::SOCKET socket) {
    auto backup_thread = std::make_shared<BackupFileSyncThread>(logger_name, ip, port, socket, locks_, local_directory_);
    backup_threads_.emplace_back(backup_thread);
    backup_thread->Start();
}

void ClientThreadPool::set_local_directory(const std::string &local_directory) {
    ClientThreadPool::local_directory_ = local_directory;
}

ClientThreadPool::~ClientThreadPool() {
    for(auto &thread : threads_) {
        thread->Join();
    }
}

void ClientThreadPool::remove_client_thread(const std::string &user_id, std::string device_id) {
    auto thread_iterator = std::find_if(threads_.begin(), threads_.end(),
                                        [&user_id, &device_id]
                                                (std::shared_ptr<ClientThread> &thread) -> bool
                                        {return thread->getDeviceId() == device_id && thread->getUserId() == user_id;}
    );
    if (thread_iterator != threads_.end()) {
        (*thread_iterator)->Join();
        threads_.erase(thread_iterator);
        disconnect_client_callback_(user_id, device_id);
    }
}


void ClientThreadPool::remove_client_thread_wrapper(ClientThreadPool &pool, const std::string &user_id,
                                                    const std::string &device_id) {
    pool.remove_client_thread(user_id, device_id);
}

void ClientThreadPool::setDisconnectClientCallback(std::function<void(std::string, std::string)> &disconnect_client_callback) {
    ClientThreadPool::disconnect_client_callback_ = disconnect_client_callback;
}
