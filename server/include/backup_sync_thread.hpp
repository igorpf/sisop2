#ifndef SISOP2_SERVER_INCLUDE_BACKUP_SYNC_THREAD_HPP
#define SISOP2_SERVER_INCLUDE_BACKUP_SYNC_THREAD_HPP

#include <spdlog/spdlog.h>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/logger_wrapper.hpp"

/**
 * Classe que encapsula a thread de sincronização do servidor primário com os servidores de backup
 * Nessa thread são enviados somente dados de clientes, não seus arquivos
 */
class BackupSyncThread : public PThreadWrapper {
public:
    BackupSyncThread(bool& is_primary, pthread_mutex_t& socket_mutex, dropbox_util::SOCKET& socket,
                     pthread_mutex_t& replica_managers_mutex, std::vector<dropbox_util::replica_manager>& replica_managers,
                     pthread_mutex_t& clients_buffer_mutex, std::vector<dropbox_util::client_info>& clients_buffer);

protected:
    /**
     * Função que executa a sincronização em intervalos pré-definidos de sync_interval_in_microseconds_
     */
    void Run() override;

private:
    int64_t sync_interval_in_microseconds_ = 50000;

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;

    bool& is_primary_;

    pthread_mutex_t& socket_mutex_;
    dropbox_util::SOCKET& socket_;

    pthread_mutex_t& replica_managers_mutex_;
    std::vector<dropbox_util::replica_manager>& replica_managers_;

    pthread_mutex_t& clients_buffer_mutex_;
    std::vector<dropbox_util::client_info>& clients_buffer_;
};

#endif //SISOP2_SERVER_INCLUDE_BACKUP_SYNC_THREAD_HPP
