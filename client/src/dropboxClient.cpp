#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxClient.hpp"

#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../include/login_command_parser.hpp"

const std::string Client::LOGGER_NAME = "Client";

Client::Client()
{
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
    logger_->set_level(spdlog::level::debug);
}

Client::~Client()
{
    spdlog::drop(LOGGER_NAME);
}

void Client::start_client(int argc, char **argv)
{
    if (logged_in_)
        return;

    LoginCommandParser login_command_parser;

    login_command_parser.ParseInput(argc, argv);

    if (login_command_parser.ShowHelpMessage())
        exit(0);

    login_command_parser.ValidateInput();

    port_ = login_command_parser.GetPort();
    hostname_ = login_command_parser.GetHostname();
    user_id_ = login_command_parser.GetUserid();

    login_server();
}

void Client::login_server()
{
    if((socket_ = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        logger_->error("Error creating socket");
        throw std::runtime_error("Error trying to login to server");
    }

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    server_addr_.sin_addr.s_addr = inet_addr(hostname_.c_str());
    peer_length_ = sizeof(server_addr_);
    std::string command("connect ");
    command.append(user_id_)
            .append(" ")
            .append(std::to_string(device_id_));

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr*)&server_addr_, peer_length_);
    logged_in_ = true;
}

void Client::sync_client()
{
    throw std::logic_error("Function not implemented");
}

void Client::send_file(const std::string& filename)
{
    util::file_transfer_request request;
    request.in_file_path = filename;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command("upload ");
    command.append(filename);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);
    util::File file_util;
    file_util.send_file(request);
}

void Client::get_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void Client::delete_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

std::vector<std::string> Client::list_server()
{
    throw std::logic_error("Function not implemented");
}

std::vector<std::string> Client::list_client()
{
    throw std::logic_error("Function not implemented");
}

void Client::close_session()
{
    throw std::logic_error("Function not implemented");
}
