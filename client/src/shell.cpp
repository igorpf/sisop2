#include "../include/shell.hpp"

#include <boost/algorithm/string.hpp>

#include "../include/shell_command_parser.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/table_printer.hpp"
#include "../../util/include/LoggerFactory.hpp"

const std::string Shell::LOGGER_NAME = "Shell";
const std::string Shell::STDOUT_LOGGER_NAME = "ClientShell";

Shell::Shell(IClient &client) : client_(client), logger_(LOGGER_NAME), stdout_logger_(STDOUT_LOGGER_NAME, true) {}

void Shell::loop(std::istream& input_stream)
{
    std::string input;
    std::vector<std::string> arguments;
    ShellCommandParser command_parser;

    do {
        try {
            std::cout << "> ";
            std::getline(input_stream, input);
            boost::split(arguments, input, boost::is_any_of(" "));

            if (input.empty())
                continue;

            command_parser.ParseInput(arguments);
            command_parser.ValidateInput();

            operation_ = command_parser.GetOperation();

            if (operation_ == "help") {
                command_parser.ShowHelpMessage();
                continue;
            }

            if (operation_ == "upload" || operation_ == "download" || operation_ == "remove")
                file_path_ = command_parser.GetFilePath();

            execute_operation();

        } catch (const std::exception& e) {
            stdout_logger_->error(e.what());
        }
    } while (operation_ != "exit");

    client_.close_session();
}

void Shell::execute_operation()
{
    if (operation_ == "upload")
        operation_upload();
    else if (operation_ == "download")
        operation_download();
    else if (operation_ == "remove")
        operation_remove();
    else if (operation_ == "list_server")
        operation_list_server();
    else if (operation_ == "list_client")
        operation_list_client();
    else if (operation_ == "get_sync_dir")
        operation_sync_dir();
}

void Shell::operation_upload()
{
    logger_->info("Sending file " + file_path_ + " to server");
    client_.send_file(file_path_);
}

void Shell::operation_download()
{
    logger_->info("Getting file " + file_path_ + " from server");
    client_.get_file(file_path_);
}

void Shell::operation_remove()
{
    logger_->info("Removing file " + file_path_ + " from server");
    client_.delete_file(file_path_);
}

void Shell::operation_list_server()
{
    logger_->info("Listing files on server");
    show_file_list(client_.list_server());
}

void Shell::operation_list_client()
{
    logger_->info("Listing files on client");
    show_file_list(client_.list_client());
}

void Shell::operation_sync_dir()
{
    logger_->info("Creating sync_dir folder");
    client_.sync_client();
}

void Shell::show_file_list(std::vector<std::vector<std::string>> file_list)
{
    // Formata tamanho e timestamp ignorando o header
    if (file_list.size() > 1) {
        for (int64_t i = 1; i < file_list.size(); ++i) {
            auto& info = file_list[i];
            info[1] = StringFormatter() << info[1] << " B";
            std::time_t timestamp = std::stoi(info[2]);
            std::tm *ptm = std::localtime(&timestamp);
            char readable_timestamp[50];
            std::strftime(readable_timestamp, 50, "%Y-%m-%d %H:%M:%S", ptm);
            info[2] = readable_timestamp;
        }
    }

    // Ordena os arquivos em ordem alfabética de nome, ignorando o header
    if (file_list.size() > 2)
        std::sort(file_list.begin() + 1, file_list.end(),
                  [] (std::vector<std::string> line_1, std::vector<std::string> line_2) ->
                          bool {return line_1[0] < line_2[0];});

    // Exibe na tela
    TablePrinter table_printer(file_list);
    table_printer.Print();
}
