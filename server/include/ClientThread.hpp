#ifndef DROPBOX_CLIENTTHREAD_HPP
#define DROPBOX_CLIENTTHREAD_HPP

#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/dropboxUtil.hpp"

/**
 * Logger name = ClientThread + user_id + device_id position in array
 */
class ClientThread : public PThreadWrapper {
public:
    ClientThread(const std::string &logger_name, const std::string &ip, int32_t port);

    ~ClientThread() override;

protected:
    void Run() override;
private:
    std::string logger_name_;
    std::shared_ptr<spdlog::logger> logger_;

    std::string ip_;
    int32_t port_;

    struct sockaddr_in client_addr_{0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;

    void start_socket();
};

#endif //DROPBOX_CLIENTTHREAD_HPP
