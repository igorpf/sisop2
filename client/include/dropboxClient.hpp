#ifndef SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
#define SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP

#include <string>
#include <vector>

#include "../../util/include/dropboxUtil.hpp"

typedef struct {
#include <netinet/in.h>
#include <spdlog/spdlog.h>

namespace util = DropboxUtil;

typedef struct client {
    bool logged_in;
    //TODO(jfguimaraes) Como identificar um novo dispositivo?
    int64_t devices[2];
    std::string user_id;
    std::vector<util::file_info> user_files;
} client;

/**
 * Initiates the client parsing the command line arguments, logging into the server
 * and spawning the background syncing thread, throws if one of the steps fails
 * @param argc Program's argument count
 * @param argv Program's argument vector
 */
void start_client(int argc, char **argv);

/**
 * Estabelece uma conexão entre o cliente e o servidor
 * @param host Endereço do servidor
 * @param port Porta de acesso ao servidor
 */
void login_server(const std::string& host, int port);
class Client {
public:

    Client(uint64_t device_id, const std::string &user_id);

    virtual ~Client();

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

#endif // SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
private:
    bool logged_in_;
    uint64_t device_id_;
    std::string user_id_;
    std::vector<util::file_info> user_files_;

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    int32_t port_;
    struct sockaddr_in server_addr_;
    util::SOCKET socket_;
    socklen_t peer_length_;
};

#endif // SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_H
