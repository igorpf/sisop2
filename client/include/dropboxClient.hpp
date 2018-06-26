#ifndef SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP
#define SISOP2_CLIENT_INCLUDE_DROPBOXCLIENT_HPP

#include <string>
#include <vector>
#include <pthread.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>

#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/logger_wrapper.hpp"
#include "iclient.hpp"

class Client : public IClient {
public:
    Client();

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

    void change_primary_server_address(std::string ip, int64_t port) override;

private:
    /**
     * Verifica se existe um id salvo no disco e o utiliza
     * Se não existir gera um novo e salva no disco
     */
    void set_device_id();

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

    /**
     * Atualiza os metadados de um arquivo modificado
     * Isso é necessário porque às vezes o evento de modificação do arquivo ocorre antes
     * dos metadados do arquivo terem sido atualizados em disco, o que causa dados incorretos no buffer
     * Deve ser chamada no sync_client para cada arquivo modificado
     */
    void update_modified_info(dropbox_util::file_info& info);

    std::string device_id_;
    std::string user_id_;

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;

    const std::string device_id_file_ = ".device_id";

    int64_t frontend_port_;
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
