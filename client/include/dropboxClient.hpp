#ifndef SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
#define SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP

#include <string>
#include <vector>
#include <pthread.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>

#include "../../util/include/dropboxUtil.hpp"
#include "iclient.hpp"

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
     */
    void send_file(const std::string &complete_file_path) override;

    /**
     * Obtém um arquivo do servidor (download)
     * @param filename Nome do arquivo a ser obtido
     */
    void get_file(const std::string &filename) override;

    /**
     * Exclui um arquivo de "sync_dir_<user_id>"
     * @param filename Nome do arquivo a ser excluído
     */
    void delete_file(const std::string &filename) override;

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
    /**
     * Estabelece uma conexão entre o cliente e o servidor
     */
    void login_server();

    /**
     * Carrega informações de arquivos já disponíveis no disco
     * Útil para quando o cliente é reiniciado
     */
    void load_info_from_disk();

    /**
     * Envia um comando para o servidor e aguarda uma mensagem de ACK, joga uma
     * exceção se não receber o ack ou se receber uma mensagem de erro
     */
    void send_command_and_expect_confirmation(const std::string& command);

    std::string device_id_;
    std::string user_id_;
    std::vector<dropbox_util::file_info> user_files_;
    std::vector<dropbox_util::file_info> modified_files_;

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    int64_t port_;
    std::string hostname_;
    struct sockaddr_in server_addr_;
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;

    pthread_mutex_t socket_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t user_files_mutex_ = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t modification_buffer_mutex_ = PTHREAD_MUTEX_INITIALIZER;
};

#endif // SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
