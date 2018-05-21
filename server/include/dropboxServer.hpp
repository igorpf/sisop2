#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP

#include "../../util/include/dropboxUtil.hpp"

#include <string>
#include <vector>

#include <netinet/in.h>
#include <spdlog/spdlog.h>

struct client_info {
    std::string user_id;
    std::vector<std::string> user_devices;
    std::vector<DropboxUtil::file_info> user_files;
};

class Server {
public:
    // TODO Implementar função pra logoff do cliente ou parar de contar quantos clientes estão online (?)
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

    /**
     * Recebe um arquivo do cliente (upload)
     * @param filename Nome do arquivo a ser recebido
     */
    void receive_file(const std::string& filename, const std::string &user_id);

    /**
     * Envia um arquivo para o cliente (download)
     * @param filename Nome do arquivo a ser enviado
     */
    void send_file(const std::string& filename, const std::string &user_id);

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

    // command related methods
    void parse_command(const std::string &command_line);
    void add_client(const std::string& user_id, const std::string& device_id);

    // utility methods
    bool has_client_connected(const std::string &client_id);
    std::vector<client_info>::iterator get_client_info(const std::string& user_id);

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    bool has_started_;
    int32_t port_;
    struct sockaddr_in server_addr_ {0};
    struct sockaddr_in current_client_ {0};
    DropboxUtil::SOCKET socket_;
    socklen_t peer_length_;
    std::vector<client_info> clients_;
    std::string local_directory_;
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
