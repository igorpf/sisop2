#ifndef DROPBOX_FRONTENDTHREAD_HPP
#define DROPBOX_FRONTENDTHREAD_HPP

#include <string>
#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/logger_wrapper.hpp"
#include "iclient.hpp"

/**
 * Thread class responsible for notifying Client class when the Primary server address
 * changes
 */

class FrontendThread : public PThreadWrapper {
public:
    FrontendThread(IClient& client_);
    void parse_command_line_input(int argc, char** argv);

protected:
    void Run() override;

private:
    IClient& client_;

    dropbox_util::SOCKET socket_;
    struct sockaddr_in server_addr_ {0};
    socklen_t peer_length_;
    int64_t port_;

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;

    void init_socket();
};


#endif //DROPBOX_FRONTENDTHREAD_HPP
