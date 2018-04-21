#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP

#include "../../util/include/dropboxUtil.hpp"

#include <string>
#include <vector>

#include <netinet/in.h>
#include <spdlog/spdlog.h>

namespace util = DropboxUtil;

typedef struct {
    bool logged_in;
    //TODO(jfguimaraes) Como identificar um novo dispositivo?
    std::vector<uint64_t> devices;
    std::string user_id;
    std::vector<util::file_info> user_files;
} client;

class Server {
public:
    Server();

    virtual ~Server();

    /**
     * Initializes server params and binds it to the given port
     * @param port Server port number (must be between 0 and 65535). Defaults to DEFAULT_SERVER_PORT
     */
    void start(int32_t port = util::DEFAULT_SERVER_PORT);

    /**
     * Starts to listen for commands from the client
     */
    void listen();

    /**
     * Sincroniza o servidor com o diretório "sync_dir_<user_id>"
     * TODO(jfguimaraes) Como a função sabe o user_id?
     */
    void sync_server();

    /**
     * Recebe um arquivo do cliente (upload)
     * @param filename Nome do arquivo a ser recebido
     * TODO(jfguimaraes) Onde salvar o arquivo? Sempre no diretório root? Ou recebe essa informação do cliente?
     */
    void receive_file(const std::string& filename);

    /**
     * Envia um arquivo para o cliente (download)
     * @param filename Nome do arquivo a ser enviado
     * TODO(jfguimaraes) Acho que deveríamos utilizar um campo fonte e um destino pra todas as transferências de arquivos
     */
    void send_file(const std::string& filename);

private:
    // command related methods
    void parse_command(const std::string &command_line);
    void add_client(const std::string &client_id, uint64_t device_id);

    // utility methods
    bool has_client_connected(const std::string &client_id);

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    bool has_started_;
    int32_t port_;
    struct sockaddr_in server_addr_;
    util::SOCKET socket_;
    socklen_t peer_length_;
    std::vector<client> clients_;
};

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_HPP
