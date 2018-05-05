#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H

#include "../../util/include/dropboxUtil.hpp"

#include <string>

#include <netinet/in.h>
#include <vector>
#include <set>

typedef struct {
    bool logged_in;
    //TODO(jfguimaraes) Como identificar um novo dispositivo?
//    uint64_t devices[2];
    std::set<uint64_t> devices;
    std::string user_id;
    std::vector<file_info> user_files;
} client;

class Server {
public:
    Server();

    /**
     * Initializes server params and binds it to the given port
     * @param port Server port number (must be between 0 and 65535). Defaults to DEFAULT_SERVER_PORT
     */
    void start(int32_t port = DEFAULT_SERVER_PORT);

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

    /**
     * True if the server has been successfully started, false otherwise
     * */
    bool has_started_;
    int32_t port_;
    struct sockaddr_in server_addr_;
    SOCKET socket_;
    int peer_length_;
    std::vector<client> clients_;
    std::shared_ptr<spdlog::logger> logger_;
};



//test functions
void start_server();

#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H
