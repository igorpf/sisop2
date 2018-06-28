#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/File.hpp"
#include "../../util/include/lock_guard.hpp"
#include "../include/dropboxServer.hpp"
#include "../include/ClientThread.hpp"
#include "../include/server_login_parser.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include <boost/filesystem.hpp>

namespace util = dropbox_util;
namespace fs = boost::filesystem;

const std::string Server::LOGGER_NAME = "Server";

Server::Server() : logger_(LOGGER_NAME, false, false) {
    std::function<void(std::string, std::string)> callback = std::bind(remove_device_from_user_wrapper, std::ref(*this), std::placeholders::_1, std::placeholders::_2);
    thread_pool_.setDisconnectClientCallback(callback);
}

void Server::start(int argc, char **argv) {
    // Cria a pasta raíz do servidor na home do usuário
    char* home_folder;

    if ((home_folder = getenv("HOME")) == nullptr) {
        home_folder = getpwuid(getuid())->pw_dir;
    }

    local_directory_ = StringFormatter() << home_folder << "/dropbox_server";
    fs::create_directory(local_directory_);
    thread_pool_.set_local_directory(local_directory_);

    load_info_from_disk();

    ServerLoginParser loginParser;
    loginParser.ParseInput(argc, argv);
    loginParser.ValidateInput();
    is_primary_ = loginParser.isPrimary();

    if ((socket_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(util::get_errno_with_message("Error initializing socket"));

    memset((void *) &server_addr_, 0, sizeof(struct sockaddr_in));
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(loginParser.GetPort()));
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    peer_length_ = sizeof(server_addr_);
    port_ = static_cast<int32_t>(loginParser.GetPort());

    if (bind(socket_, (struct sockaddr *) &server_addr_, peer_length_) == util::DEFAULT_ERROR_CODE)
        throw std::runtime_error(util::get_errno_with_message("Bind error"));
    logger_->info("Initialized socket of number {} for server", socket_);

    if (!is_primary_) {
        primary_server_port_ = loginParser.GetPrimaryServerPort();
        primary_server_ip = loginParser.GetPrimaryServerIp();
        logger_->info("This is a backup server. Registering to primary {} {}", primary_server_ip, primary_server_port_);
        register_in_primary_server();
        serverConnectivityDetectorThread.setPrimaryServerIp(primary_server_ip);
        serverConnectivityDetectorThread.setPrimaryServerPort(primary_server_port_);
        serverConnectivityDetectorThread.Start();
        serverConnectivityDetectorThread.setNotifyCallback(
            std::bind(
                [&](Server &s) -> void {
                    // start election here
                    logger_->info("Received notification that the primary server is down");
                    // if this server gets elected, stop the detector thread
                    auto new_manager = replica_managers_[0];
                    if (new_manager.port == port_) { //this is the new server
                        serverConnectivityDetectorThread.stop();
                        notify_new_elected_server_to_clients();
                    } else {
                        // point to new primary server
                        primary_server_port_ = new_manager.port;
                        primary_server_ip = new_manager.ip;
                        serverConnectivityDetectorThread.setPrimaryServerIp(primary_server_ip);
                        serverConnectivityDetectorThread.setPrimaryServerPort(primary_server_port_);
                    }
                }, std::ref(*this))
        );
    }

    has_started_ = true;
}

void Server::load_info_from_disk() {
    for (const fs::directory_entry& entry : fs::directory_iterator(local_directory_)) {
        if (!fs::is_directory(entry.path())) {
            throw std::runtime_error("Unexpected file on server root folder");
        }

        std::string user_id = entry.path().filename().string();
        add_client(user_id);

        auto client_iterator = get_client_info(user_id);

        for (const fs::directory_entry& file : fs::directory_iterator(entry.path())) {
            if (fs::is_directory(file.path())) {
                throw std::runtime_error(StringFormatter() << "Unexpected subfolder on user " << user_id << " folder");
            }

            std::string user_file = file.path().string();
            std::string filename_without_path = file.path().filename().string();

            util::file_info user_file_info;
            user_file_info.name = filename_without_path;
            user_file_info.size = fs::file_size(user_file);
            user_file_info.last_modification_time = fs::last_write_time(user_file);

            client_iterator->user_files.emplace_back(user_file_info);
        }
        save_client_change_to_buffer(user_id);
    }
}

void Server::listen() {
    if (!has_started_)
        return;
    logger_->info("Server is listening on port {}", port_);
    char buffer[util::BUFFER_SIZE];
    while (true) {
        try {
            std::fill(buffer, buffer + sizeof(buffer), 0);
            LockGuard socket_lock(socket_mutex_);
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &current_client_, &peer_length_);
            socket_lock.Unlock();
            logger_->debug("Received from client {} port {} the message: {}",
                           inet_ntoa(current_client_.sin_addr), ntohs(current_client_.sin_port), buffer);
            parse_command(buffer);
            // TODO Remover
            sync_backup();
        } catch (std::runtime_error& runtime_error) {
            logger_->error("Error parsing command from client {}", runtime_error.what());
            break;
        } catch (std::logic_error& logic_error) {
            logger_->error("Error parsing command from client {}, client {} ip {}", logic_error.what());
            send_command_error_message(current_client_, logic_error.what());
        }
    }
}

void Server::parse_command(const std::string &command_line) {
    logger_->debug("Parsing command {}", command_line);
    auto tokens = util::split_words_by_token(command_line);
    auto command = tokens[0];
    if (command == "connect") {
        auto user_id = tokens[1], device_id = tokens[2];
        auto frontend_port = static_cast<int64_t >(std::stoi(tokens[3]));
        login_new_client(user_id, device_id, frontend_port);
//        notify_new_elected_server_to_clients();
    }
    else if (command == "backup") {
        add_backup_server();
    }
    else if (command == dropbox_util::CHECK_PRIMARY_SERVER_MESSAGE) {
        send_command_confirmation(current_client_);
    }
    else if (command == "replica_list") {
//        send_command_confirmation(current_client_);
        tokens.erase(tokens.begin());
        parse_replica_list(tokens);
    }
    else if (command == "backup_sync") {
//        send_command_confirmation(current_client_);
        parse_backup_list(tokens[1]);
    }
    else {
        throw std::logic_error(StringFormatter() << "Invalid command sent by client " << command_line);
    }
}

void Server::parse_replica_list(std::vector<std::string> replicas) {
    replica_managers_.clear();
    for (auto &replica : replicas) {
        logger_->info("Adding new replica {}", replica);
        auto tokens = dropbox_util::split_words_by_token(replica, "@");
        replica_managers_.emplace_back(replica_manager{tokens[0], std::stoi(tokens[1])});
    }
}

void Server::parse_backup_list(const std::string &client_infos) {
    std::cout << "Received info:" << std::endl;
    std::cout << client_infos << std::endl;
    auto clients = dropbox_util::split_words_by_token(client_infos, "@");

    for (const auto& client_info : clients) {
        auto tokens = dropbox_util::split_words_by_token(client_info, "%");
        std::cout << "user id: " << tokens[0] << std::endl;
        add_client(tokens[0]);

        if (tokens.size() == 1)
            continue;

        auto client_iterator = get_client_info(tokens[0]);
        auto devices_and_files = dropbox_util::split_words_by_token(tokens[1], "&");

        for (const auto& elements : devices_and_files) {
            if (elements[0] == '#') {
                // Files
                std::cout << "User files:" << std::endl;
                auto files = dropbox_util::split_words_by_token(elements.substr(1, elements.size() - 1), ":");
                for (const auto& file : files) {
                    auto fields = dropbox_util::split_words_by_token(file, ",");
                    std::cout << "File name: " << fields[0] << std::endl;
                    std::cout << "File size: " << fields[1] << std::endl;
                    std::cout << "File timestamp: " << fields[2] << std::endl;
                }
            } else if (elements[0] == '$') {
                // Devices
                std::cout << "User devices:" << std::endl;
                auto devices = dropbox_util::split_words_by_token(elements.substr(1, elements.size() - 1), ":");
                for (const auto& device : devices) {
                    auto fields = dropbox_util::split_words_by_token(device, ",");
                    std::cout << "Device id: " << fields[0] << std::endl;
                    std::cout << "Device ip: " << fields[1] << std::endl;
                    std::cout << "Device port: " << fields[2] << std::endl;
                    std::cout << "Device frontend port: " << fields[3] << std::endl;
                }
            } else {
                throw std::runtime_error("Invalid type of client info received from primary server!");
            }
        }
    }
}

void Server::add_client(const std::string &user_id) {
    if(!has_client_connected(user_id)) {
        dropbox_util::client_info new_client;
        new_client.user_id = user_id;
        clients_.emplace_back(new_client);
        clients_buffer_.emplace_back(new_client);
        std::string client_path = StringFormatter() << local_directory_ << "/" << user_id;
        fs::create_directory(client_path);
    }
}

void Server::send_command_confirmation(struct sockaddr_in &client) {
    LockGuard socket_lock(socket_mutex_);
    sendto(socket_, "ACK", 4, 0, (struct sockaddr *)&client, peer_length_);
}

void Server::send_command_error_message(struct sockaddr_in &client, const std::string &error_message)  {
    const std::string complete_error_message = StringFormatter() << util::ERROR_MESSAGE_INITIAL_TOKEN << error_message;
    LockGuard socket_lock(socket_mutex_);
    sendto(socket_, complete_error_message.c_str(), complete_error_message.size(), 0, (struct sockaddr *)&client, peer_length_);
}

void Server::login_new_client(const std::string &user_id, const std::string &device_id, int64_t frontend_port) {
    if (!has_client_and_device_connected(user_id, device_id)) {
        add_client(user_id);
        auto client_iterator = get_client_info(user_id);
        if (client_iterator->user_devices.size() >= MAX_CLIENT_DEVICES)
            throw std::logic_error("User achieved maximum devices connected");
        std::string client_ip = inet_ntoa(current_client_.sin_addr);
        int32_t client_port = ntohs(current_client_.sin_port);

        dropbox_util::device new_device{device_id, client_ip, client_port, frontend_port};
        client_iterator->user_devices.emplace_back(new_device);

        save_client_change_to_buffer(user_id);

        auto new_client_connection = allocate_connection_for_client(client_ip);
        dropbox_util::new_client_param_list param_list{
            user_id,
            device_id,
            "ClientThread_" + user_id + "_" + device_id,
            client_ip,
            client_port,
            new_client_connection.socket,
            *client_iterator,
            frontend_port,
            clients_buffer_mutex_,
            clients_buffer_
        };
        thread_pool_.add_client(param_list);
        send_command_confirmation(current_client_);
        std::string port = std::to_string(new_client_connection.port);
        sendto(socket_, port.c_str(), port.size(), 0, (struct sockaddr*)&current_client_, peer_length_);

        logger_->info("Connected new device for client, total clients: {}. User_id {} device_id {} frontend_port {}",
                      clients_.size(), user_id, device_id, frontend_port);
    }
    else
        throw std::logic_error("User has already connected with this device");
}

new_client_connection_info Server::allocate_connection_for_client(const std::string &ip) {
    dropbox_util::SOCKET new_socket;
    next_client_port_++;

    if ((new_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        throw std::runtime_error(dropbox_util::get_errno_with_message("Error initializing socket"));

    struct sockaddr_in client_addr_{0};
    client_addr_.sin_family = AF_INET;
    client_addr_.sin_addr.s_addr = INADDR_ANY;
    client_addr_.sin_port = htons(static_cast<uint16_t>(next_client_port_));

    while(bind(new_socket, (struct sockaddr *) &client_addr_, sizeof(client_addr_)) == dropbox_util::DEFAULT_ERROR_CODE) {
        next_client_port_++;
        client_addr_.sin_port = htons(static_cast<uint16_t>(next_client_port_));
        if (next_client_port_ > dropbox_util::MAX_VALID_PORT) {
            throw std::runtime_error("Could not find new port for the client");
        }
    }
    logger_->debug("Found new connection for client. Socket {}, port {}, ip {}", new_socket, next_client_port_, ip);

    new_client_connection_info new_client{new_socket, next_client_port_};
    return new_client;
}

bool Server::has_client_connected(const std::string &client_id) {
    auto client_iterator = get_client_info(client_id);
    return client_iterator != clients_.end();
}

bool Server::has_client_and_device_connected(const std::string &client_id, const std::string &device_id) {
    auto client_iterator = get_client_info(client_id);
    if (client_iterator == clients_.end())
        return false;
    auto device_iterator = std::find_if(client_iterator->user_devices.begin(), client_iterator->user_devices.end(),
                                        [&device_id] (const dropbox_util::device& dev) -> bool {return dev.device_id == device_id;});
    return device_iterator != client_iterator->user_devices.end();
}

std::vector<dropbox_util::client_info>::iterator Server::get_client_info(const std::string& user_id) {
    return std::find_if(clients_.begin(), clients_.end(),
                        [&user_id] (const dropbox_util::client_info& c) -> bool {return user_id == c.user_id;});
}

void Server::remove_device_from_user(const std::string &user_id, std::string device_id) {
    auto client_iterator = get_client_info(user_id);
    if (client_iterator != clients_.end()) {
        auto device_iterator = std::find_if(
                client_iterator->user_devices.begin(), client_iterator->user_devices.end(),
                [&device_id] (const dropbox_util::device& device) -> bool {return device.device_id == device_id;}
        );
        if (device_iterator != client_iterator->user_devices.end()) {
            client_iterator->user_devices.erase(device_iterator);
            save_client_change_to_buffer(user_id);
        }
    }
}

void Server::remove_device_from_user_wrapper(Server &server, const std::string &user_id, const std::string &device_id) {
    server.remove_device_from_user(user_id, device_id);
}

void Server::register_in_primary_server() {
    struct sockaddr_in primary_server_addr {0};
    primary_server_addr.sin_family = AF_INET;
    primary_server_addr.sin_port = htons(static_cast<uint16_t>(primary_server_port_));
    primary_server_addr.sin_addr.s_addr = inet_addr(primary_server_ip.c_str());
    std::string command = StringFormatter() << "backup" << dropbox_util::COMMAND_SEPARATOR_TOKEN << "123";
    sendto(socket_, command.c_str() , command.size(), 0, (struct sockaddr *)&primary_server_addr, sizeof(primary_server_addr));
}

void Server::save_client_change_to_buffer(const std::string& user_id) {
    auto client_iterator = get_client_info(user_id);

    // Remove old change from the buffer
    clients_buffer_.erase(std::remove_if(clients_buffer_.begin(), clients_buffer_.end(),
                                     [&user_id] (const dropbox_util::client_info &c) -> bool
                                     {return user_id == c.user_id;}),
                          clients_buffer_.end());

    // Add to the buffer
    clients_buffer_.emplace_back(*client_iterator);
}

void Server::add_backup_server() {
    std::string ip = inet_ntoa(current_client_.sin_addr);
    int64_t port = ntohs(current_client_.sin_port);
    replica_manager new_manager {ip, port};
    replica_managers_.emplace_back(new_manager);
//    replica_managers_buffer_.emplace_back(new_manager);

    send_replica_manager_list();
}

void Server::send_replica_manager_list() const {
    std::string replica_command = StringFormatter() << "replica_list" << dropbox_util::COMMAND_SEPARATOR_TOKEN;

    for (auto &manager : replica_managers_) {
        replica_command.append(StringFormatter() << manager.ip << "@" << manager.port << dropbox_util::COMMAND_SEPARATOR_TOKEN);
    }

    replica_command.pop_back();

    LockGuard socket_lock(socket_mutex_);

    for (auto &manager : replica_managers_) {
        struct sockaddr_in replica_addr {0};
        replica_addr.sin_family = AF_INET;
        replica_addr.sin_port = htons(static_cast<uint16_t>(manager.port));
        replica_addr.sin_addr.s_addr = inet_addr(manager.ip.c_str());

        sendto(socket_, replica_command.c_str() , replica_command.size(), 0, (struct sockaddr *)&replica_addr, sizeof(replica_addr));
    }
}

void Server::notify_new_elected_server_to_clients() {
    std::this_thread::sleep_for(std::chrono::microseconds(1000000));

    LockGuard socket_lock(socket_mutex_);

    for (auto &client : clients_) {
        for (auto &device : client.user_devices) {
            logger_->info("Notifying client {} device  ip {} port {}", client.user_id, device.ip, device.frontend_port);
            if(device.ip.empty() || device.frontend_port == 0)
                continue;
            auto new_client_connection = allocate_connection_for_client(device.ip);
            dropbox_util::new_client_param_list param_list{
                    client.user_id,
                    device.device_id,
                    "ClientThread_" + client.user_id + "_" + device.device_id,
                    device.ip,
                    ntohs(current_client_.sin_port),
                    new_client_connection.socket,
                    client,
                    device.frontend_port,
                    clients_buffer_mutex_,
                    clients_buffer_
            };
            thread_pool_.add_client(param_list);
            struct sockaddr_in primary_server_addr {0};
            primary_server_addr.sin_family = AF_INET;
            primary_server_addr.sin_port = htons(static_cast<uint16_t>(device.frontend_port));
            primary_server_addr.sin_addr.s_addr = inet_addr(device.ip.c_str());
            std::string command = StringFormatter() << "new_server" << dropbox_util::COMMAND_SEPARATOR_TOKEN << new_client_connection.port;
            sendto(socket_, command.c_str() , command.size(), 0, (struct sockaddr *)&primary_server_addr, sizeof(primary_server_addr));
        }
    }
}

void Server::sync_backup() {
    LockGuard client_buffer_lock(clients_buffer_mutex_);

    if (!clients_buffer_.empty()) {
        std::string backup_command = StringFormatter() << "backup_sync" << dropbox_util::COMMAND_SEPARATOR_TOKEN;

        for (const auto &info : clients_buffer_) {
            backup_command.append(info.user_id);

            if (!info.user_devices.empty() or !info.user_files.empty())
                backup_command.append(dropbox_util::USER_INFO_SEPARATOR_TOKEN);

            if(!info.user_devices.empty()) {
                backup_command.append(dropbox_util::DEVICE_INITIAL_TOKEN);
                for (const auto &device : info.user_devices) {
                    backup_command.append(StringFormatter() << device.device_id << ","
                                                            << device.ip << "," << device.port << ","
                                                            << device.frontend_port << ":");
                }
                backup_command.pop_back();
                backup_command.append(dropbox_util::DEVICE_FILE_SEPARATOR_TOKEN);
            }

            if (!info.user_files.empty()) {
                backup_command.append(dropbox_util::FILE_INITIAL_TOKEN);
                for (const auto &file : info.user_files) {
                    backup_command.append(StringFormatter() << file.name << ","
                                                            << file.size << ","
                                                            << file.last_modification_time << ":");
                }
                backup_command.pop_back();
            }

            backup_command.append(dropbox_util::USER_LIST_SEPARATOR_TOKEN);
        }

        backup_command.pop_back();
        logger_->info(backup_command);

        clients_buffer_.clear();

        client_buffer_lock.Unlock();

        LockGuard socket_lock(socket_mutex_);

        for (auto &manager : replica_managers_) {
            struct sockaddr_in replica_addr{0};
            replica_addr.sin_family = AF_INET;
            replica_addr.sin_port = htons(static_cast<uint16_t>(manager.port));
            replica_addr.sin_addr.s_addr = inet_addr(manager.ip.c_str());

            sendto(socket_, backup_command.c_str(), backup_command.size(), 0, (struct sockaddr *) &replica_addr,
                   sizeof(replica_addr));
        }
    }
}
