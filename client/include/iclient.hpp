#ifndef SISOP2_CLIENT_INCLUDE_ICLIENT_HPP
#define SISOP2_CLIENT_INCLUDE_ICLIENT_HPP

#include <string>
#include <vector>

#include "../../util/include/dropboxUtil.hpp"

/**
 * Interface genérica para um objeto do tipo cliente
 * Usada para facilitar o teste de módulos que interagem com a classe Client
 */
class IClient {
public:
    virtual ~IClient() = default;

    virtual void sync_client() = 0;

    virtual void send_file(const std::string &filename) = 0;
    virtual void get_file(const std::string &filename) = 0;
    virtual void delete_file(const std::string& filename) = 0;

    virtual void change_primary_server_address(std::string ip, int64_t port) = 0;

    virtual std::vector<std::vector<std::string>> list_server() = 0;
    virtual std::vector<std::vector<std::string>> list_client() = 0;

    virtual void close_session() = 0;

    bool logged_in_ = false;
    std::string local_directory_;

    std::vector<dropbox_util::file_info> user_files_;
    std::vector<dropbox_util::file_info> modified_files_;

    pthread_mutex_t user_files_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t modification_buffer_mutex_ = PTHREAD_MUTEX_INITIALIZER;
};

#endif // SISOP2_CLIENT_INCLUDE_ICLIENT_HPP
