#ifndef DROPBOX_PRIMARYSERVERCONNECTIVITYDETECTORTHREAD_HPP
#define DROPBOX_PRIMARYSERVERCONNECTIVITYDETECTORTHREAD_HPP

#include <functional>
#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/logger_wrapper.hpp"

/**
 * Class dedicated to check if the primary server
 * is reachable. Meant to be run by a backup server
 */

class PrimaryServerConnectivityDetectorThread : public PThreadWrapper {
public:
    PrimaryServerConnectivityDetectorThread();

    void setNotifyCallback(const std::function<void()> &notify_primary_server_disconnection_callback__);

    void setPrimaryServerIp(const std::string &primary_server_ip_);

    void setPrimaryServerPort(int64_t primary_server_port_);

    void stop();

    void pause_for_election();

    void proceed_after_election();

protected:
    void Run() override;

private:
    void init_server_address();

    std::function<void()> notify_primary_server_disconnection_callback__;

    const int8_t MAX_RETRIES = 10;

    const int64_t verify_interval_in_microsseconds = 500;

    LoggerWrapper logger_;

    /**
     * Indicates whether the server is still a backup. If the backup server using this thread
     * gets elected as a primary server, this thread should stop running.
     */
    bool is_backup_ = true;

    bool is_election_happening_ = false;
    std::string primary_server_ip_;
    int64_t primary_server_port_;

    struct sockaddr_in primary_server_addr{0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;
};


#endif //DROPBOX_PRIMARYSERVERCONNECTIVITYDETECTORTHREAD_HPP
