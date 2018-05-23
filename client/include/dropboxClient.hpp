#ifndef SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
#define SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP

#include <string>
#include <vector>
#include <netinet/in.h>
#include <spdlog/spdlog.h>

#include "../../util/include/dropboxUtil.hpp"
#include "iclient.hpp"

namespace util = DropboxUtil;

class Client : public IClient {
public:
    Client();
    ~Client() override;

    /**
     * Inicializa o client fazendo o parse dos argumentos de linha de comando,
     * fazendo o login no servidor com essas informações e gerando o device id
     */
    void start_client(int argc, char **argv);

    /**
     * Sincroniza o diretório "sync_dir_<user_id>" com o servidor
     */
    void sync_client() override;

    /**
     * Envia um arquivo para o servidor (upload)
     * @param complete_file_path Nome do arquivo a ser enviado
     * TODO(jfguimaraes) O nome do arquivo é um caminho absoluto ou relativo?
     */
    void send_file(const std::string &complete_file_path) override;

    /**
     * Obtém um arquivo do servidor (download)
     * @param filename Nome do arquivo a ser obtido
     * TODO(jfguimaraes) É possível otimizar copiando o arquivo do diretório sync_dir local?
     */
    void get_file(const std::string &filename) override;

    /**
     * Exclui um arquivo de "sync_dir_<user_id>"
     * @param filename Nome do arquivo a ser excluído
     */
    void delete_file(const std::string &filename);

    /**
     * Lista os arquivos do usuário no servidor
     */
    std::vector<std::vector<std::string>> list_server() override;

    /**
     * Lista os arquivos do usuário na pasta local
     */
    std::vector<std::vector<std::string>> list_client() override;

    /**
     * Fecha a sessão com o servidor
     */
    void close_session() override;

private:
    bool logged_in_ = false;
    std::string device_id_;
    std::string user_id_;
    std::vector<util::file_info> user_files_;
    std::string local_directory_;

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    int64_t port_;
    std::string hostname_;
    struct sockaddr_in server_addr_;
    util::SOCKET socket_;
    socklen_t peer_length_;

    /**
     * Estabelece uma conexão entre o cliente e o servidor
     */
    void login_server();
};

#endif // SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
