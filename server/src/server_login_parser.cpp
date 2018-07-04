#include "../include/server_login_parser.hpp"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

ServerLoginParser::ServerLoginParser() {
    description_.add_options()
            ("help,h", "shows help message")
            ("type", program_options::value<std::string>(), "server type (primary or backup)")
            ("port", program_options::value<int64_t>(), "server port")
            ("primary_ip", program_options::value<std::string>(), "primary server ip (should be specified only when starting as a backup)")
            ("primary_port", program_options::value<int64_t>(), "primary server port (should be specified only when starting as a backup)")
            ;

    positional_description_.add("type", 1);
    positional_description_.add("port", 1);
    positional_description_.add("primary_ip", 1);
    positional_description_.add("primary_port", 1);
}

void ServerLoginParser::ParseInput(int argc, char **argv) {
    program_options::store(program_options::command_line_parser(argc, argv)
                                   .options(description_)
                                   .positional(positional_description_)
                                   .run(),
                           variables_map_);

    bool help_specified = variables_map_.count("help") > 0;
    bool server_info_specified = variables_map_.count("type") > 0 &&
                                 variables_map_.count("port") > 0;

    if (!help_specified && !server_info_specified)
        throw std::runtime_error("missing parameters, use --help or -h for usage info");
}

void ServerLoginParser::ValidateInput() {
    if (variables_map_.empty())
        throw std::runtime_error("No arguments have been parsed");

    if (variables_map_.count("help") > 0)
        return;

    auto type = variables_map_["type"].as<std::string>();
    if (type != "primary" && type != "backup")
        throw std::runtime_error("Invalid type for server");

    auto port = variables_map_["port"].as<int64_t>();
    if (port < 0 || port > 65536)
        throw std::runtime_error("Invalid port");

    if (type == "backup") {
        if(variables_map_.count("primary_port") == 0 || variables_map_.count("primary_ip") == 0)
            throw std::runtime_error("Must specify primary server port and ip for registration purposes");

        auto primary_port = variables_map_["primary_port"].as<int64_t>();
        if (primary_port < 0 || primary_port > 65536)
            throw std::runtime_error("Invalid primary port");

        auto primary_ip = variables_map_["primary_ip"].as<std::string>();
        boost::system::error_code error_code;
        boost::asio::ip::make_address(primary_ip, error_code);

        if (error_code)
            throw std::runtime_error("Invalid primary_ip: " + error_code.message());
    }

}

bool ServerLoginParser::ShowHelpMessage() {
    if (variables_map_.empty())
        throw std::runtime_error("No arguments have been parsed");

    if (variables_map_.count("help") == 0)
        return false;

    std::cout << "Usage: ./dropboxServer [--help] type port\n";
    std::cout << description_ << std::endl;

    return true;
}

bool ServerLoginParser::isPrimary() {
    if (variables_map_.count("type") == 0)
        throw std::runtime_error("No type available");

    auto type = variables_map_["type"].as<std::string>();
    return type == "primary";
}

int64_t ServerLoginParser::GetPort() {
    if (variables_map_.count("port") == 0)
        throw std::runtime_error("No port available");

    return variables_map_["port"].as<int64_t >();
}

int64_t ServerLoginParser::GetPrimaryServerPort() {
    if (variables_map_.count("primary_port") == 0)
        throw std::runtime_error("No port available");

    return variables_map_["primary_port"].as<int64_t >();
}

std::string ServerLoginParser::GetPrimaryServerIp() {
    if (variables_map_.count("primary_ip") == 0)
        throw std::runtime_error("No port available");

    return variables_map_["primary_ip"].as<std::string>();
}


