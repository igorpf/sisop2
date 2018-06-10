#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/LoggerFactory.hpp"

#include <string>
#include <vector>

#include <netinet/in.h>
#include <spdlog/spdlog.h>

struct client_info {
    std::string user_id;
    std::vector<std::string> user_devices;
    std::vector<dropbox_util::file_info> user_files;
};

class Server {
public:
    // TODO Implementar função pra logoff do cliente que remove o dispositivo da lista (é isso que deve acontecer?)
    // TODO Se um comando falha o próximo sync do cliente falha por falta de ACK, revisar isso
    Server();
    virtual ~Server();

    /**
     * Inicializa o servidor e o coloca para ouvir comandos dos clientes
     */
    void listen();

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
     * Verifica o comando enviado pelo cliente e executa a operação correspondente
     */
    void parse_command(const std::string& command_line);

    /**
     * Envia uma confirmação ao cliente de que um comando foi recebido
     */
    void send_command_confirmation();

    /**
     * Envia uma mensagem de erro ao cliente
     */
    void send_command_error_message(const std::string &error_message);

    /**
     * Adiciona um novo cliente e seu respectivo dispositivo à lista de clientes
     * Se o cliente já está na lista o novo dispositivo é adicionado
     * TODO Enviar mensagem de erro quando o cliente já estiver com o número máximo de dispositivos
     */
    void add_client(const std::string& user_id, const std::string& device_id);

    /**
     * Verifica se o cliente já está na lista de usuários
     */
    bool has_client_connected(const std::string &client_id);

    /**
     * Retorna uma referência (na forma de um iterador) para as informações do usuário na lista
     */
    std::vector<client_info>::iterator get_client_info(const std::string& user_id);

    /**
     * Remove o arquivo da lista de arquivos do cliente
     */
    void remove_file_from_client(const std::string& user_id, const std::string& filename);

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
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
