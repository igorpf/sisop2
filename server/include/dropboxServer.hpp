#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP

#include "../../util/include/dropboxUtil.hpp"
#include "ClientThreadPool.hpp"
#include "../../util/include/LoggerFactory.hpp"
#include "../../util/include/logger_wrapper.hpp"

#include <string>
#include <vector>

#include <netinet/in.h>
#include <spdlog/spdlog.h>

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
    void login_new_client(const std::string &user_id, const std::string &device_id);

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

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;

    const int64_t MAX_CLIENT_DEVICES = 2;

    bool has_started_;
    bool is_primary_;
    int64_t primary_server_port_;
    std::string primary_server_ip;

    int32_t port_;
    int32_t next_client_port_ = dropbox_util::DEFAULT_SERVER_PORT;
    struct sockaddr_in server_addr_ {0};
    struct sockaddr_in current_client_ {0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;
    std::vector<dropbox_util::client_info> clients_;
    std::string local_directory_;

    ClientThreadPool thread_pool_;
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
