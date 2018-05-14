#include "../include/shell.hpp"

#include <boost/algorithm/string.hpp>

#include "../include/shell_command_parser.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/table_printer.hpp"

const std::string Shell::LOGGER_NAME = "Shell";

Shell::Shell(IClient &client) : client_(client)
{
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
}

Shell::~Shell()
{
    spdlog::drop(LOGGER_NAME);
}

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

            if (operation_ == "upload" || operation_ == "download")
                file_path_ = command_parser.GetFilePath();

            execute_operation();

        } catch (const std::exception& e) {
            logger_->error(e.what());
        }
    } while (operation_ != "exit");

    client_.close_session();
}

void Shell::execute_operation()
{
    // TODO Use a map to map the strings to the functions
    if (operation_ == "upload") {
        operation_upload();
    } else if (operation_ == "download") {
        operation_download();
    } else if (operation_ == "list_server") {
        operation_list_server();
    } else if (operation_ == "list_client") {
        operation_list_client();
    } else if (operation_ == "get_sync_dir") {
        operation_sync_dir();
    }
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

void Shell::operation_list_server()
{
    logger_->info("Listing files on server");
    std::vector<std::vector<std::string>> server_files = client_.list_server();
    TablePrinter table_printer(server_files);
    table_printer.Print();
}

void Shell::operation_list_client()
{
    logger_->info("Listing files on client");
    std::vector<std::vector<std::string>> client_files = client_.list_client();
    TablePrinter table_printer(client_files);
    table_printer.Print();
}

void Shell::operation_sync_dir()
{
    logger_->info("Creating sync_dir folder");
    client_.sync_client();
}
