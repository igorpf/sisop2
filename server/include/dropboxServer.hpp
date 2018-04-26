#ifndef SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H
#define SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H

#include "../../util/include/dropboxUtil.hpp"

#include <string>

#include <netinet/in.h>

const uint16_t DEFAULT_SERVER_PORT = 9001;

class Server {
public:

    /**
     * Initializes server params and binds it to the given port
     * @param port Server port number (must be between 0 and 65535). Defaults to DEFAULT_SERVER_PORT
     */
    void start(uint16_t port = DEFAULT_SERVER_PORT);

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
    /**
     * True if the server has been successfully started, false otherwise
     * */
    bool has_started_;
    uint16_t port_;
    struct sockaddr_in server_addr_;
    SOCKET socket_;
    int peer_length_;
};


#endif // SISOP2_SERVER_INCLUDE_DROPBOXSERVER_H
