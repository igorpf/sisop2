#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP

#include <string>
#include <vector>
#include <netinet/in.h>
#include <spdlog/spdlog.h>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/LoggerFactory.hpp"
#include "../../util/include/logger_wrapper.hpp"

#include "PrimaryServerConnectivityDetectorThread.hpp"
#include "ClientThreadPool.hpp"
#include "backup_sync_thread.hpp"

struct new_client_connection_info {
    dropbox_util::SOCKET socket;
    int32_t port;
};

class Server {
public:
    Server();

    /**
     * Coloca o servidor para ouvir comandos dos clientes
     */
    void listen();

    /**
     * Inicializa as estruturas internas do servidor
     */
    void start(int argc, char **argv);
private:

    /**
     * Inicializa as informações dos clientes com base no que já está em disco
     * Útil para quando o servidor é reiniciado
     */
    void load_info_from_disk();

    /**
     * Verifica o comando enviado pelo cliente e executa a operação correspondente
     */
    void parse_command(const std::string &command_line);

    /**
     * Spawns a new thread to watch for the client's requests
     */
    void login_new_client(const std::string &user_id, const std::string &device_id, int64_t frontend_port);

    /**
     * Adds new client in memory
     */
    void add_client(const std::string &user_id);

    /**
     * Checks if the client is already on the list of clients
     */
    bool has_client_connected(const std::string &client_id);

    bool has_client_and_device_connected(const std::string &client_id, const std::string &device_id);

    void remove_device_from_user(const std::string &user_id, std::string device_id);
    static void remove_device_from_user_wrapper(Server &server, const std::string &user_id, const std::string &device_id);

    /**
     * Returns the client info inside the list of client infos
     */
    std::vector<dropbox_util::client_info>::iterator get_client_info(const std::string& user_id);

    void send_command_confirmation(struct sockaddr_in &client);
    void send_command_error_message(struct sockaddr_in &client, const std::string &error_message);

    new_client_connection_info allocate_connection_for_client(const std::string &ip);

    /**
     * Register this server to the primary server. Should be called only when this server instance is a backup
     */
    void register_in_primary_server();

    /**
     * Saves the client info change on the buffer to be replicated to backup servers
     */
    void save_client_change_to_buffer(const std::string& user_id);

    void add_backup_server();

    void notify_new_elected_server_to_clients();

    void send_replica_manager_list() const;

    void parse_replica_list(std::vector<std::string> replicas);

    void parse_backup_list(const std::string& client_info_list);

    void send_command_and_expect_confirmation(const std::string& command);

    void get_file(const std::string& filename, const std::string& user_id);

    void start_election();

    void continue_election(int64_t id);

    void notify_elected_server_to_next_participant(int64_t id);

    dropbox_util::replica_manager get_next_replica_in_ring();

    dropbox_util::replica_manager get_new_primary_server(int64_t id);

    void remove_new_primary_server_from_backup_list(int64_t id);

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;

    const int64_t MAX_CLIENT_DEVICES = 2;

    bool has_started_;
    bool is_primary_;
    int64_t primary_server_port_;
    std::string primary_server_ip;
    PrimaryServerConnectivityDetectorThread serverConnectivityDetectorThread;

    bool has_participated_in_election_ = false;
    int64_t id_;


    int32_t port_;
    int32_t next_client_port_ = dropbox_util::DEFAULT_SERVER_PORT;
    struct sockaddr_in server_addr_ {0};
    struct sockaddr_in primary_server_thread_addr_ {0};
    struct sockaddr_in current_client_ {0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;

    mutable pthread_mutex_t socket_mutex_ = PTHREAD_MUTEX_INITIALIZER;

    std::string local_directory_;
    ClientThreadPool thread_pool_;
    BackupSyncThread backup_sync_thread_;

    std::vector<dropbox_util::client_info> clients_;

    mutable pthread_mutex_t replica_managers_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    std::vector<dropbox_util::replica_manager> replica_managers_;

    pthread_mutex_t clients_buffer_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    std::vector<dropbox_util::client_info> clients_buffer_;
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
