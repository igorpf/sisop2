#include "../include/shell_command_parser.hpp"

#include <iostream>
#include <exception>

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

namespace fs = boost::filesystem;

ShellCommandParser::ShellCommandParser() {
    description_.add_options()
            ("operation", program_options::value<std::string>(),
                    "Operation to execute, available operations:\n\n"
                    "help                \tshows the help message\n"
                    "upload <filepath>   \tuploads the file on filepath to the server\n"
                    "download <filepath> \tdownloads the file on the server's filepath\n"
                    "remove <filepath>   \tremoves the file on the server's filepath\n"
                    "list_server         \tlists the files on the server\n"
                    "list_client         \tlists the files on the sync_dir_userid local folder\n"
                    "get_sync_dir        \tcreates the directory sync_dir_userid on your home folder\n"
                    "exit                \tcloses the connection to the server\n")
            ("filepath", program_options::value<std::string>(), "path to the file on which to operate (applies to upload, download and remove)\n")
            ;

    positional_description_.add("operation", 1);
    positional_description_.add("filepath", 1);
}

void ShellCommandParser::ParseInput(std::vector<std::string> arguments) {
    variables_map_.clear();

    program_options::store(program_options::command_line_parser(arguments)
                                   .options(description_)
                                   .positional(positional_description_)
                                   .run(),
                           variables_map_);

    bool operation_specified = variables_map_.count("operation") > 0;

    if (!operation_specified)
        throw std::runtime_error("Missing parameters, use help for usage info");

    auto operation = variables_map_["operation"].as<std::string>();
    bool path_used = operation == "upload" || operation == "download" || operation == "remove";

    bool path_specified_when_should = path_used ? variables_map_.count("filepath") > 0 : true;

    if (!path_specified_when_should)
        throw std::runtime_error("Missing parameters, use help for usage info");

    bool path_not_specified_when_shouldnt = !path_used ? variables_map_.count("filepath") < 1 : true;

    if (!path_not_specified_when_shouldnt)
        throw std::runtime_error("Too many parameters, use help for usage info");
}

void ShellCommandParser::ValidateInput() {
    if (variables_map_.empty())
        throw std::runtime_error("No arguments have been parsed");

    auto operation = variables_map_["operation"].as<std::string>();

    if (std::find(available_operations.begin(), available_operations.end(), operation) == available_operations.end())
        throw std::runtime_error("Invalid operation, use help for usage info");

    if (operation == "upload") {
        fs::path file_path {variables_map_["filepath"].as<std::string>().c_str()};

        if (!fs::exists(file_path))
            throw std::runtime_error("Local file doesn't exist");
        else if (fs::is_directory(file_path))
            throw std::runtime_error("Upload of directories is not supported");
    }
}

void ShellCommandParser::ShowHelpMessage() {
    if (variables_map_.empty())
        throw std::runtime_error("No arguments have been parsed");

    std::cout << "Usage: operation filepath\n";
    std::cout << description_ << std::endl;
}

std::string ShellCommandParser::GetOperation() {
    if (variables_map_.count("operation") == 0)
        throw std::runtime_error("No operation available");

    return variables_map_["operation"].as<std::string>();
}

std::string ShellCommandParser::GetFilePath() {
    if (variables_map_.count("filepath") == 0)
        throw std::runtime_error("No filepath available");

    return variables_map_["filepath"].as<std::string>();
}
