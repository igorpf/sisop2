#include "../include/login_command_parser.hpp"

#include <iostream>
#include <exception>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

LoginCommandParser::LoginCommandParser() {
    description_.add_options()
            ("help,h", "shows help message")
            ("userid", program_options::value<std::string>(), "user identifier")
            ("hostname", program_options::value<std::string>(), "server hostname")
            ("port", program_options::value<int64_t>(), "server port")
            ;

    positional_description_.add("userid", 1);
    positional_description_.add("hostname", 1);
    positional_description_.add("port", 1);
}

void LoginCommandParser::ParseInput(int argc, char **argv) {
    program_options::store(program_options::command_line_parser(argc, argv)
                                   .options(description_)
                                   .positional(positional_description_)
                                   .run(),
                           variables_map_);

    bool help_specified = variables_map_.count("help") > 0;
    bool client_info_specified = variables_map_.count("userid") > 0 &&
                                 variables_map_.count("hostname") > 0 &&
                                 variables_map_.count("port") > 0;

    if (!help_specified && !client_info_specified)
        throw std::runtime_error("missing parameters, use --help or -h for usage info");
}

void LoginCommandParser::ValidateInput() {
    if (variables_map_.empty())
        throw std::runtime_error("No arguments have been parsed");

    if (variables_map_.count("help") > 0)
        return;

    auto userid = variables_map_["userid"].as<std::string>();

    if (userid.size() < 4)
        throw std::runtime_error("User id must have more than 4 characters");

    auto hostname = variables_map_["hostname"].as<std::string>();
    boost::system::error_code error_code;
    boost::asio::ip::make_address(hostname, error_code);

    if (error_code)
        throw std::runtime_error("Invalid hostname: " + error_code.message());

    auto port = variables_map_["port"].as<int64_t>();

    if (port < 0 || port > 65536)
        throw std::runtime_error("Invalid port");
}

bool LoginCommandParser::ShowHelpMessage() {
    if (variables_map_.empty())
        throw std::runtime_error("No arguments have been parsed");

    if (variables_map_.count("help") == 0)
        return false;

    std::cout << "Usage: dropboxClient [--help] userid hostname port\n";
    std::cout << description_ << std::endl;

    return true;
}

std::string LoginCommandParser::GetUserid() {
    if (variables_map_.count("userid") == 0)
        throw std::runtime_error("No userid available");

    return variables_map_["userid"].as<std::string>();
}

std::string LoginCommandParser::GetHostname() {
    if (variables_map_.count("hostname") == 0)
        throw std::runtime_error("No hostname available");

    return variables_map_["hostname"].as<std::string>();
}

int64_t LoginCommandParser::GetPort() {
    if (variables_map_.count("port") == 0)
        throw std::runtime_error("No port available");

    return variables_map_["port"].as<int64_t >();
}
