#include <iostream>
#include <cstring>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>

#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "../../util/include/string_formatter.hpp"
#include "../../util/include/table_printer.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxClient.hpp"

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

    boost::uuids::name_generator_sha1 id_generator(boost::uuids::ns::dns());
    boost::uuids::uuid device_id = id_generator(user_id_);
    device_id_ = to_string(device_id);

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
    std::string command(StringFormatter() << "connect" << util::COMMAND_SEPARATOR_TOKEN
                                          << user_id_ << util::COMMAND_SEPARATOR_TOKEN << device_id_);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr*)&server_addr_, peer_length_);
    logged_in_ = true;
}

void Client::sync_client()
{
    if (local_directory_.empty()) {
        char *home_folder;

        if ((home_folder = getenv("HOME")) == nullptr) {
            home_folder = getpwuid(getuid())->pw_dir;
        }

        local_directory_ = StringFormatter() << home_folder << "/sync_dir_" << user_id_;
        boost::filesystem::create_directory(local_directory_);
    }

    // TODO Sync files with the server
    // throw std::logic_error("Function not implemented");
}

void Client::send_file(const std::string& filename)
{
    util::file_transfer_request request;
    request.in_file_path = filename;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command(StringFormatter() << "upload" << util::COMMAND_SEPARATOR_TOKEN << filename);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);

    util::File file_util;
    file_util.send_file(request);
}

void Client::get_file(const std::string& filename)
{
    util::file_transfer_request request;
    request.in_file_path = StringFormatter() << local_directory_ << "/" << filename;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command(StringFormatter() << "download " << filename << " " << user_id_);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);

    util::File file_util;
    file_util.receive_file(request);
}

void Client::delete_file(const std::string& filename)
{
    // TODO Server should answer if it succeeded
    std::string command(StringFormatter() << "delete " << filename << " " << user_id_);
    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);
}

std::vector<std::vector<std::string>> Client::list_server()
{
    util::file_transfer_request request;
    request.peer_length = peer_length_;
    request.server_address = server_addr_;
    request.socket = socket_;

    std::string command(StringFormatter() << "list_server " << user_id_);

    sendto(socket_, command.c_str(), command.size(), 0, (struct sockaddr *)&server_addr_, peer_length_);

    util::File file_util;
    return file_util.receive_list_files(request);
}

std::vector<std::vector<std::string>> Client::list_client()
{
    std::vector<std::vector<std::string>> entries;
    std::vector<std::string> header {"name", "size", "modification_time"};
    entries.emplace_back(header);

    for (const auto& file : user_files_) {
        std::vector<std::string> info;
        info.emplace_back(file.name);
        info.emplace_back(std::to_string(file.size));
        info.emplace_back(std::to_string(file.last_modification_time));
        entries.emplace_back(info);
    }

    return entries;
}

void Client::close_session()
{
    throw std::logic_error("Function not implemented");
}
