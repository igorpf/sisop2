#ifndef SISOP2_CLIENT_INCLUDE_SHELL_COMMAND_PARSER_HPP
#define SISOP2_CLIENT_INCLUDE_SHELL_COMMAND_PARSER_HPP

#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace program_options = boost::program_options;

/**
 * Classe que interpreta os comandos do shell
 */
class ShellCommandParser {
public:
    /**
     * Inicializa a descrição dos argumentos
     */
    ShellCommandParser();

    /**
     * Verifica que a entrada está completa e no formato correto
     */
    void ParseInput(std::vector<std::string> arguments);

    /**
     * Verifica que o comando é válido
     */
    void ValidateInput();

    /**
     * Exibe a mensagem de ajuda, gera uma exceção se ParseInput não foi chamado
     */
    void ShowHelpMessage();

    /**
     * Retorna a operação especificada, gera uma exceção se ParseInput não foi chamado
     */
    std::string GetOperation();

    /**
     * Retorna o caminho do arquivo especificado se aplicável,
     * gera uma exceção se ParseInput não foi chamado ou se não houver um caminho disponível
     */
    std::string GetFilePath();

private:
    program_options::options_description description_;
    program_options::positional_options_description positional_description_;
    program_options::variables_map variables_map_;

    std::array<std::string, 8> available_operations = {{"help", "upload", "download", "remove", "list_server",
                                                       "list_client", "get_sync_dir", "exit"}};
};

#endif // SISOP2_CLIENT_INCLUDE_SHELL_COMMAND_PARSER_HPP
