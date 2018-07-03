#include "../include/backup_file_sync_thread.hpp"

#include <arpa/inet.h>
#include <boost/filesystem.hpp>

#include "../../util/include/string_formatter.hpp"
#include "../../util/include/lock_guard.hpp"
#include "../../util/include/File.hpp"

namespace fs = boost::filesystem;

BackupFileSyncThread::BackupFileSyncThread(const std::string& logger_name, const std::string& ip, int64_t port,
                                           dropbox_util::SOCKET socket, const std::map<std::string, pthread_mutex_t>& locks,
                                           const std::string& local_directory) : logger_name_(logger_name), logger_(logger_name),
                                                                                 ip_(ip), port_(port), socket_(socket),
                                                                                 locks_(locks), local_directory_(local_directory)
{}

void BackupFileSyncThread::Run() {
    logger_->info("New backup file sync thread started, socket {} ip {}, port {}", socket_, ip_, port_);
    set_backup_server_address();
    char buffer[dropbox_util::BUFFER_SIZE];
    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            logger_->debug("Waiting message from backup server {} port {}",
                           inet_ntoa(backup_addr_.sin_addr), ntohs(backup_addr_.sin_port));
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &backup_addr_, &peer_length_);
            logger_->debug("Received from backup server {} port {} the message: {}",
                           inet_ntoa(backup_addr_.sin_addr), ntohs(backup_addr_.sin_port), buffer);
            parse_command(buffer);
        } catch (std::runtime_error& runtime_error) {
            logger_->error("Error parsing command from backup server {}", runtime_error.what());
            break;
        } catch (std::logic_error& logic_error) {
            logger_->error("Error parsing command from backup server {}", logic_error.what());
            send_command_error_message(logic_error.what());
        } catch (std::exception &e) {
            logger_->info("Error parsing command from backup server {}", e.what());
            break;
        }
    }
}

void BackupFileSyncThread::set_backup_server_address() {
    backup_addr_.sin_family = AF_INET;
    backup_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    backup_addr_.sin_addr.s_addr = inet_addr(ip_.c_str());
    peer_length_ = sizeof(backup_addr_);
}

void BackupFileSyncThread::parse_command(const std::string &command_line) {
    logger_->debug("Parsing command {}", command_line);
    auto tokens = dropbox_util::split_words_by_token(command_line);
    auto command = tokens[0];
    if (command == "download") {
        send_file(tokens[1], tokens[2]);
    } else {
        throw std::logic_error(StringFormatter() << "Invalid command sent by backup server " << command_line);
    }
}

void BackupFileSyncThread::send_file(const std::string& filename, const std::string& user_id) {
    dropbox_util::file_transfer_request request;
    request.socket = socket_;
    request.server_address = backup_addr_;
    request.peer_length = peer_length_;

    fs::path filepath(filename);
    std::string filename_without_path = filepath.filename().string();

    request.in_file_path = StringFormatter() << local_directory_ << "/" << user_id << "/" << filename_without_path;

    if(!dropbox_util::File::file_exists(request.in_file_path))
        throw std::logic_error("The requested file does not exist!");

    send_command_confirmation();

    if (locks_.count(user_id) > 0) {
        LockGuard client_lock(locks_[user_id]);
        dropbox_util::File file_util;
        file_util.send_file(request);
    } else {
        dropbox_util::File file_util;
        file_util.send_file(request);
    }
}

void BackupFileSyncThread::send_command_confirmation() {
    sendto(socket_, "ACK", 4, 0, (struct sockaddr *) &backup_addr_, peer_length_);
}

void BackupFileSyncThread::send_command_error_message(const std::string& error_message)  {
    const std::string complete_error_message = StringFormatter() << dropbox_util::ERROR_MESSAGE_INITIAL_TOKEN << error_message;
    sendto(socket_, complete_error_message.c_str(), complete_error_message.size(), 0, (struct sockaddr *) &backup_addr_, peer_length_);
}
