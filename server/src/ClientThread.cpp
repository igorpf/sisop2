#include <iostream>
#include <arpa/inet.h>
#include <boost/filesystem/path.hpp>
#include "../include/ClientThread.hpp"
#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/File.hpp"
#include "../../util/include/LoggerFactory.hpp"

namespace fs = boost::filesystem;

ClientThread::ClientThread(const std::string &local_directory, const std::string &logger_name,
                           const std::string &ip, int32_t port, dropbox_util::SOCKET socket, dropbox_util::client_info &info) :
        local_directory_(local_directory), logger_name_(logger_name), ip_(ip), port_(port), socket_(socket), info_(info) {
    logger_ = LoggerFactory::getLoggerForName(logger_name);
}

ClientThread::~ClientThread() {
    spdlog::drop(logger_name_);
}

void ClientThread::parse_command(const std::string &command_line) {
    // TODO Validar parâmetros
    logger_->debug("Parsing command {}", command_line);
    auto tokens = dropbox_util::split_words_by_token(command_line);
    auto command = tokens[0];
    if (command == "download") {
        send_file(tokens[1], tokens[2]);
    } else if (command == "upload") {
        send_command_confirmation();
        receive_file(tokens[1], tokens[2]);
    } else if (command == "remove") {
        delete_file(tokens[1], tokens[2]);
        send_command_confirmation();
    } else if (command == "list_server") {
        send_command_confirmation();
        list_server(tokens[1]);
    }
    else {
        throw std::logic_error(StringFormatter() << "Invalid command sent by client " << command_line);
    }
}

void ClientThread::send_command_confirmation() {
    sendto(socket_, "ACK", 4, 0, (struct sockaddr *)&client_addr_, peer_length_);
}

void ClientThread::Run() {
    logger_->info("New client thread started, socket {} ip {}, port {}", socket_, ip_, port_);
    char buffer[dropbox_util::BUFFER_SIZE];
    init_client_address();
    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            logger_->debug("Waiting message from client {} port {}",
                           inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port));
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr_, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port), buffer);
            parse_command(buffer);
        } catch (std::runtime_error& runtime_error) {
            logger_->error("Error parsing command from client {}", runtime_error.what());
            break;
        } catch (std::logic_error& logic_error) {
            logger_->error("Error parsing command from client {}", logic_error.what());
            send_command_error_message(logic_error.what());
        }
    }
}

void ClientThread::send_command_error_message(const std::string &error_message)  {
    const std::string complete_error_message = StringFormatter() << dropbox_util::ERROR_MESSAGE_INITIAL_TOKEN << error_message;
    sendto(socket_, complete_error_message.c_str(), complete_error_message.size(), 0, (struct sockaddr *)&client_addr_, peer_length_);
}

void ClientThread::init_client_address() {
    client_addr_.sin_family = AF_INET;
    client_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    client_addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    peer_length_ = sizeof(client_addr_);
}

void ClientThread::send_file(const std::string& filename, const std::string &user_id) {
    dropbox_util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = client_addr_;
    request.peer_length = peer_length_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    request.in_file_path = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;

    if(!dropbox_util::File::file_exists(request.in_file_path))
        throw std::logic_error("The requested file does not exist!");

    send_command_confirmation();
    dropbox_util::File file_util;
    file_util.send_file(request);
}

void ClientThread::receive_file(const std::string& filename, const std::string &user_id) {
    dropbox_util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = client_addr_;
    request.peer_length = peer_length_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();
    std::string local_file_path = StringFormatter()
            << local_directory_ << "/" << user_id << "/" << filename_without_path;

    request.in_file_path = local_file_path;

    dropbox_util::File file_util;
    file_util.receive_file(request);

    dropbox_util::file_info received_file_info;
    received_file_info.name = filename_without_path;
    received_file_info.size = fs::file_size(local_file_path);
    received_file_info.last_modification_time = fs::last_write_time(local_file_path);


    remove_file_from_info(filename);
    add_file(received_file_info);
}

void ClientThread::add_file(const dropbox_util::file_info &received_file_info) {
    // TODO put a lock guard here
    info_.user_files.emplace_back(received_file_info);
}


void ClientThread::delete_file(const std::string& filename, const std::string &user_id) {
    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    std::string server_file = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;
    fs::remove(server_file);

    remove_file_from_info(filename_without_path);
}


void ClientThread::list_server(const std::string &user_id) {
    std::string user_file_list {"name;size;modification_time&"};

    // TODO put a guard here
    for (const auto& file : info_.user_files) {
        user_file_list.append(StringFormatter() << file.name << ';'<< file.size << ';'
                                                << file.last_modification_time << '&');
    }

    // Remove último &
    user_file_list.pop_back();

    dropbox_util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = client_addr_;
    request.peer_length = peer_length_;

    dropbox_util::File file_util;
    file_util.send_list_files(request, user_file_list);
}

void ClientThread::remove_file_from_info(const std::string &filename) {
    // TODO put a lock guard here
    if (!info_.user_files.empty())
        info_.user_files.erase(std::remove_if(info_.user_files.begin(),
                                              info_.user_files.end(),
                                                         [&filename] (const dropbox_util::file_info& info) ->
                                                                 bool {return filename == info.name;}),
                               info_.user_files.end());
}

