#include "../include/shell.hpp"

#include <iostream>
#include <boost/algorithm/string.hpp>

#include "../include/shell_command_parser.hpp"

const std::string Shell::LOGGER_NAME = "Shell";

Shell::Shell(IClient &client) : client_(client)
{
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
}

void Shell::loop()
{
    std::string input;
    std::vector<std::string> arguments;
    ShellCommandParser command_parser;

    do {
        try {
            std::cout << "> ";
            std::getline(std::cin, input);
            boost::split(arguments, input, boost::is_any_of(" "));

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
}

void Shell::execute_operation()
{
    if (operation_ == "upload") {
        std::cout << "Sending file " << file_path_ << " to server" << std::endl;
        client_.send_file(file_path_);
    } else if (operation_ == "download") {
        std::cout << "Getting file " << file_path_ << " from server" << std::endl;
        client_.get_file(file_path_);
    } else if (operation_ == "list_server") {
        std::cout << "Listing files on server" << std::endl;
        std::vector<std::string> server_files = client_.list_server();
        // TODO(jfguimaraes) Define a printer for these functions
    } else if (operation_ == "list_client") {
        std::cout << "Listing files on client" << std::endl;
        std::vector<std::string> client_files = client_.list_client();
        // TODO(jfguimaraes) Define a printer for these functions
    } else if (operation_ == "get_sync_dir") {
        std::cout << "Creating sync_dir_userid folder" << std::endl;
        client_.sync_client();
    }
}
