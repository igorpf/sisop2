#include <iostream>
#include <arpa/inet.h>
#include <boost/filesystem/path.hpp>
#include "../include/ClientThread.hpp"
#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/File.hpp"

namespace fs = boost::filesystem;

ClientThread::ClientThread(const std::string &local_directory, const std::string &logger_name,
                           const std::string &ip, int32_t port, dropbox_util::SOCKET socket) : local_directory_(local_directory),
         logger_name_(logger_name), ip_(ip), port_(port), socket_(socket) {
    logger_ = spdlog::stdout_color_mt(logger_name);
    logger_->set_level(spdlog::level::debug);
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
        receive_file(tokens[1], tokens[2]);
    } else if (command == "remove") {
        delete_file(tokens[1], tokens[2]);
    } else if (command == "list_server") {
        list_server(tokens[1]);
    }
    else {
        throw std::logic_error(StringFormatter() << "Invalid command sent by client " << command_line);
    }
}

void ClientThread::Run() {
    logger_->info("New client thread started, socket {} ip {}, port {}", socket_, ip_, port_);
    char buffer[dropbox_util::BUFFER_SIZE];

    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            logger_->debug("Waiting message from client {} port {}",
                           inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port));
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr_, &peer_length_);
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port), buffer);
            parse_command(buffer);

        } catch (std::exception& e) {
            logger_->error("Error parsing command from client {}", e.what());
            break;
        }
    }
}

void ClientThread::send_file(const std::string& filename, const std::string &user_id) {
    dropbox_util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = client_addr_;
    request.peer_length = peer_length_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    request.in_file_path = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;

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


//    auto client_iterator = get_client_info(user_id);
//
//    // Se já existe um registro do arquivo o remove para adição do novo registro
//    remove_file_from_client(user_id, filename);
//
//    client_iterator->user_files.emplace_back(received_file_info);
}


void ClientThread::delete_file(const std::string& filename, const std::string &user_id) {
    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    std::string server_file = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;
    fs::remove(server_file);

//    remove_file_from_client(user_id, filename_without_path);
}


void ClientThread::list_server(const std::string &user_id) {
//    auto client_iterator = get_client_info(user_id);
//    std::string user_file_list {"name;size;modification_time&"};
//
//    for (const auto& file : client_iterator->user_files) {
//        user_file_list.append(StringFormatter() << file.name << ';'<< file.size << ';'
//                                                << file.last_modification_time << '&');
//    }
//
//    // Remove último &
//    user_file_list.pop_back();
//
//    dropbox_util::file_transfer_request request;
//    request.socket = socket_;
//    request.server_address = client_addr_;
//    request.peer_length = peer_length_;
//
//    dropbox_util::File file_util;
//    file_util.send_list_files(request, user_file_list);
}
