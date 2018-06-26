#ifndef DROPBOX_CLIENTTHREAD_HPP
#define DROPBOX_CLIENTTHREAD_HPP

#include "../../util/include/pthread_wrapper.hpp"
#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/logger_wrapper.hpp"

struct client_thread_param_list {
    std::string user_id;
    std::string device_id;
    std::string local_directory;
    std::string logger_name;
    std::string ip;
    int32_t port;
    dropbox_util::SOCKET socket;
    dropbox_util::client_info &info;
    pthread_mutex_t &client_info_mutex;
    int64_t frontend_port;
};

/**
 * Logger name = ClientThread + user_id + device_id position in array
 */
class ClientThread : public PThreadWrapper {
public:
    ClientThread(client_thread_param_list param_list);


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

    /**
     * Replaces file in local_directory_ by temporary file if it is
     * newer than the local one (i.e has a more recent modification time)
     */
    void replace_local_file_by_temporary_if_more_recent(const std::string &tmp_file_path, const std::string &local_file_path,
                                                        const dropbox_util::file_info &received_file_info);

    void parse_command(const std::string &command_line);
    void send_command_confirmation();
    void send_command_error_message(const std::string &error_message);

    void setLogoutCallback(const std::function<void()> &logout_callback);

    const std::string &getUserId() const;

    const std::string &getDeviceId() const;

protected:
    void Run() override;
private:
    std::string logger_name_;
    LoggerWrapper logger_;

    std::string ip_;
    int32_t port_;
    int64_t frontend_port;
    std::string user_id_;
    std::string device_id_;

    struct sockaddr_in client_addr_{0};
    dropbox_util::SOCKET socket_;
    socklen_t peer_length_;

    std::string local_directory_;

    dropbox_util::client_info &info_;

    pthread_mutex_t &client_info_mutex_;
    void init_client_address();

    std::function<void()> logout_callback_;
};

#endif //DROPBOX_CLIENTTHREAD_HPP
