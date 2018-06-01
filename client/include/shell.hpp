#ifndef SISOP2_CLIENT_INCLUDE_SHELL_HPP
#define SISOP2_CLIENT_INCLUDE_SHELL_HPP

#include <iostream>
#include <spdlog/spdlog.h>

#include "iclient.hpp"

/**
 * Classe que implementa uma interface com o usuário na forma de um shell,
 * aceitando comandos em loop até que o comando de saída seja executado
 */
class Shell {
public:
    /**
     * Construtor recebe uma referência a um objeto do tipo IClient para poder
     * executar as funções passadas pelo usuário ou pelo framework de testes
     */
    explicit Shell(IClient& client);

    ~Shell();

    /**
     * Função que executa o prompt e interpreta os comandos do usuário
     * Executa até receber o comando de saída
     * @param input_stream Stream de entrada de dados, padrão std::cin
     *                     Pode ser modificado para testes
     */
    void loop(std::istream& input_stream = std::cin);

private:
    IClient& client_;
    std::string operation_;
    std::string file_path_;

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<spdlog::logger> stdout_logger_;

    /**
     * Executa a operação indicada pelo usuário
     */
    void execute_operation();

    void operation_upload();
    void operation_download();
    void operation_remove();
    void operation_list_server();
    void operation_list_client();
    void operation_sync_dir();

    /**
     * Shows the list of files in the screen, formatting sizes and timestamps
     */
    void show_file_list(std::vector<std::vector<std::string>> file_list);
};

#endif // SISOP2_CLIENT_INCLUDE_SHELL_HPP
