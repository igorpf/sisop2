#ifndef SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_H
#define SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_H

#include "../../util/include/dropboxUtil.hpp"

#include <string>
#include <vector>

#include <netinet/in.h>

typedef struct client {
    bool logged_in;
    //TODO(jfguimaraes) Como identificar um novo dispositivo?
    int64_t devices[2];
    std::string user_id;
    std::vector<file_info> user_files;
} client;

class Client {
public:

    Client(uint64_t device_id_, const std::string &user_id_);

    /**
     * Estabelece uma conexão entre o cliente e o servidor
     * @param host Endereço do servidor
     * @param port Porta de acesso ao servidor
     */
    void login_server(const std::string &host, int32_t port);

    /**
     * Sincroniza o diretório "sync_dir_<user_id>" com o servidor
     */
    void sync_client();

    /**
     * Envia um arquivo para o servidor (upload)
     * @param filename Nome do arquivo a ser enviado
     * TODO(jfguimaraes) O nome do arquivo é um caminho absoluto ou relativo?
     */
    void send_file(const std::string &filename);

    /**
     * Obtém um arquivo do servidor (download)
     * @param filename Nome do arquivo a ser obtido
     * TODO(jfguimaraes) É possível otimizar copiando o arquivo do diretório sync_dir local?
     */
    void get_file(const std::string &filename);

    /**
     * Exclui um arquivo de "sync_dir_<user_id>"
     * @param filename Nome do arquivo a ser excluído
     */
    void delete_file(const std::string &filename);

    /**
     * Fecha a sessão com o servidor
     */
    void close_session();

private:
    // attributes from specification
    bool logged_in_;
    uint64_t device_id_;
    std::string user_id_;
    std::vector<file_info> user_files_;

    // other attributes
    int32_t port_;
    struct sockaddr_in server_addr_;
    SOCKET socket_;
    socklen_t peer_length_;
    std::shared_ptr<spdlog::logger> logger_;
};

#endif // SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_H
