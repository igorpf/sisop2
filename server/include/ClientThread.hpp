#ifndef DROPBOX_CLIENTTHREAD_HPP
#define DROPBOX_CLIENTTHREAD_HPP

#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/dropboxUtil.hpp"

/**
 * Logger name = ClientThread + user_id + device_id position in array
 */
class ClientThread : public PThreadWrapper {
public:
    ClientThread(const std::string &local_directory, const std::string &logger_name, const std::string &ip, int32_t port,
                 dropbox_util::SOCKET socket, dropbox_util::client_info &info);

    ~ClientThread() override;

    /**
     * Envia um arquivo para o cliente (download)
     */
    void send_file(const std::string& filename, const std::string &user_id);

    // TODO O servidor recebe instruções de manipulação de arquivos sem timestamp, executando sempre
    // Isso pode ser um problema se dois clientes modificam um arquivo mas enviam as modificações na ordem errada
    /**
     * Recebe um arquivo do cliente (upload)
     */
    void receive_file(const std::string& filename, const std::string &user_id);

    /**
     * Remove um arquivo do cliente (remove)
     */
    void delete_file(const std::string& filename, const std::string &user_id);

    void add_file(const dropbox_util::file_info &received_file_info);

    /**
     * Envia a lista de arquivos do usuário no servidor no formato
     * nome_arquivo_1;tamanho_arquivo_1;timestamp_arquivo_1&nome_arquivo_2;tamanho_arquivo_2;timestamp_arquivo_2&...
     */
    void list_server(const std::string& user_id);

    /**
     * Sincroniza o servidor com o diretório "sync_dir_<user_id>"
     */
    void sync_server();

    /**
     * Removes the file from the list of files of the client
     */
    void remove_file_from_info(const std::string &filename);


    void parse_command(const std::string &command_line);
    void send_command_confirmation();
    void send_command_error_message(const std::string &error_message);

protected:
    void Run() override;
private:
    std::string logger_name_;
    std::shared_ptr<spdlog::logger> logger_;

    std::string ip_;
    int32_t port_;

    struct sockaddr_in client_addr_{0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;

    std::string local_directory_;
    dropbox_util::client_info &info_;

    void init_client_address();
};

#endif //DROPBOX_CLIENTTHREAD_HPP
