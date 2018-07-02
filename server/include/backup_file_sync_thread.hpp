#ifndef SISOP2_SERVER_INCLUDE_BACKUP_FILE_SYNC_THREAD_HPP
#define SISOP2_SERVER_INCLUDE_BACKUP_FILE_SYNC_THREAD_HPP

#include <spdlog/spdlog.h>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/logger_wrapper.hpp"

/**
 * Classe que encapsula a thread de sincronização dos arquivos dos clientes com os servidores de backup
 * Nessa thread são enviados somente os arquivos dos clientes, não seus dados
 */
class BackupFileSyncThread : public PThreadWrapper {
public:
    BackupFileSyncThread(const std::string& logger_name, const std::string& ip, int64_t port,
                         dropbox_util::SOCKET socket, const std::map<std::string, pthread_mutex_t>& locks,
                         const std::string& local_directory);

protected:
    /**
     * Função que executa a sincronização em intervalos pré-definidos de sync_interval_in_microseconds_
     */
    void Run() override;

private:
    void set_backup_server_address();

    void parse_command(const std::string &command_line);

    void send_file(const std::string& filename, const std::string& user_id);

    void send_command_confirmation();

    void send_command_error_message(const std::string& error_message);

    std::string logger_name_;
    LoggerWrapper logger_;

    std::string ip_;
    int64_t port_;

    struct sockaddr_in backup_addr_{0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;

    std::map<std::string, pthread_mutex_t> locks_;
    std::string local_directory_;
};

#endif //SISOP2_SERVER_INCLUDE_BACKUP_FILE_SYNC_THREAD_HPP
