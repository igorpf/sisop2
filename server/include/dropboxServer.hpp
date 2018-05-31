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

    /**
     * Sincroniza o servidor com o diretório "sync_dir_<user_id>"
     */
    void sync_server();

    // TODO O servidor recebe instruções de manipulação de arquivos sem timestamp, executando sempre
    // Isso pode ser um problema se dois clientes modificam um arquivo mas enviam as modificações na ordem errada
    /**
     * Recebe um arquivo do cliente (upload)
     */
    void receive_file(const std::string& filename, const std::string &user_id);

    /**
     * Envia um arquivo para o cliente (download)
     */
    void send_file(const std::string& filename, const std::string &user_id);

    /**
     * Remove um arquivo do cliente (remove)
     */
    void delete_file(const std::string& filename, const std::string &user_id);

    /**
     * Envia a lista de arquivos do usuário no servidor no formato
     * nome_arquivo_1;tamanho_arquivo_1;timestamp_arquivo_1&nome_arquivo_2;tamanho_arquivo_2;timestamp_arquivo_2&...
     */
     void list_server(const std::string& user_id);

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
     * Adds a new client and it's respective device to the list of clients
     * If the client is already on the list a new device is added
     * TODO Error when client already has max devices
     */
    void add_client(const std::string& user_id, const std::string& device_id);

    /**
     * Checks if the client is already on the list of clients
     */
    bool has_client_connected(const std::string &client_id);

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
    struct sockaddr_in server_addr_ {0};
    struct sockaddr_in current_client_ {0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;
    std::vector<client_info> clients_;
    std::string local_directory_;

    ClientThreadPool thread_pool_;
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
