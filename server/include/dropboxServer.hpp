#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP

#include "../../util/include/dropboxUtil.hpp"
#include "ClientThreadPool.hpp"

#include <string>
#include <vector>

#include <netinet/in.h>
#include <spdlog/spdlog.h>

struct client_info {
    std::string user_id;
    std::vector<std::string> user_devices;
    std::vector<dropbox_util::file_info> user_files;
};

struct new_client_connection_info {
    dropbox_util::SOCKET socket;
    int32_t port;
};

class Server {
public:
    // TODO Implementar função pra logoff do cliente que remove o dispositivo da lista (é isso que deve acontecer?)
    Server();

    virtual ~Server();

    /**
     * Inicializa o servidor e o coloca para ouvir comandos dos clientes
     */
    void listen();

private:
    /**
     * Inicializa as estruturas internas do servidor
     */
    void start();

    /**
     * Inicializa as informações dos clientes com base no que já está em disco
     * Útil para quando o servidor é reiniciado
     */
    void load_info_from_disk();

    /**
     * Parses the command received by the client
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

    /**
     * Returns the client info inside the list of client infos
     */
    std::vector<client_info>::iterator get_client_info(const std::string& user_id);

    /**
     * Removes the file from the list of files of the client
     */
    void remove_file_from_client(const std::string& user_id, const std::string& filename);

    new_client_connection_info allocate_connection_for_client(const std::string &ip);

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    const int64_t MAX_CLIENT_DEVICES = 2;

    bool has_started_;
    int32_t port_;
    int32_t next_client_port_ = dropbox_util::DEFAULT_SERVER_PORT;
    struct sockaddr_in server_addr_ {0};
    struct sockaddr_in current_client_ {0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;
    std::vector<client_info> clients_;
    std::string local_directory_;

    ClientThreadPool thread_pool_;
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
