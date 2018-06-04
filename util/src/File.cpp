#include "../include/dropboxUtil.hpp"
#include "../include/File.hpp"
#include "../include/LoggerFactory.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace dropbox_util;

const std::string File::LOGGER_NAME = "File";

File::File() {
    logger_ = LoggerFactory::getLoggerForName(LOGGER_NAME);
}

File::~File() {
    spdlog::drop(LOGGER_NAME);
}

int64_t get_file_length(filesystem::path &path) {
    if (!filesystem::exists(path))
        throw std::runtime_error("file doesn't exist");
    if (filesystem::is_regular_file(path))
        return static_cast<int64_t>(filesystem::file_size(path));
    else
        throw std::runtime_error("path does not point to a file");
}

void File::send_packet_with_retransmission(file_transfer_request request, struct sockaddr_in from, char* packet, size_t packet_size) {
    bool ack_error;
    int64_t received_bytes,
            retransmissions = 0;
    do {
        char ack[4]{0};
        sendto(request.socket, packet, packet_size, 0, (struct sockaddr *)&request.server_address,
               request.peer_length);
        received_bytes = recvfrom(request.socket, ack, sizeof(ack), 0,(struct sockaddr *) &from, &request.peer_length);
        ack[3] = '\0';
        ack_error = received_bytes <= 0 || strcmp(ack, "ACK") != 0;
        if(ack_error) {
            logger_->error("Error receiving ACK, retransmitting packet. Retransmissions {}", retransmissions);
            retransmissions++;
            if(retransmissions >= MAX_RETRANSMISSIONS) {
                logger_->error("Achieved maximum retransmissions of {}, aborting file transmission", retransmissions);
                throw std::runtime_error("Maximum retransmissions achieved");
            }
        }
    } while(ack_error);
}

void File::send_file(file_transfer_request request) {
    struct sockaddr_in from{};

    std::ifstream input_file;
    input_file.open(request.in_file_path.c_str(), std::ios::binary);
    if(!input_file.is_open()) {
        logger_->error("Could not open file {}", request.in_file_path);
        throw std::runtime_error("Error opening file");
    }
    enable_socket_timeout(request);

    filesystem::path path(request.in_file_path);
    int64_t file_length = get_file_length(path),
            packets = static_cast<int64_t>(std::ceil(file_length / ((float) BUFFER_SIZE))),
            sent_packets = 0,
            file_read_bytes;
    char buffer[BUFFER_SIZE] = {0};
    logger_->debug("Divided file transmission in {} packets. File size: {}", packets, file_length);

    start_handshake(request, from);
    send_file_metadata(request, from, path);

    while(packets--) {
        std::fill(buffer, buffer + sizeof(buffer), 0);
        input_file.read(buffer, sizeof(buffer));
        file_read_bytes = input_file.gcount();
        logger_->debug("Sending buffer size of: {}, already sent packets: {}", file_read_bytes, sent_packets);

        send_packet_with_retransmission(request, from, buffer, static_cast<size_t>(file_read_bytes));
        sent_packets++;
    }

    logger_->debug("Sending end of file");
    buffer[0] = EOF_SYMBOL;
    send_packet_with_retransmission(request, from, buffer, 1);
    input_file.close();

    send_finish_handshake(request, from);
    disable_socket_timeout(request);
    logger_->debug("Successfully sent file");
}

void File::receive_file(file_transfer_request request) {
    struct sockaddr_in client_addr{0};
    establish_handshake(request, client_addr);

    std::string out_path(request.in_file_path);
    std::ofstream output_file;
    output_file.open(out_path.c_str(), std::ios::binary);
    if(!output_file.is_open()) {
        logger_->error("Could not open file {}", request.in_file_path);
        throw std::runtime_error("Error opening file");
    }

    char buffer[BUFFER_SIZE] = {0}, ack[4];
    int64_t received_bytes;
    received_bytes = recvfrom(request.socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &request.peer_length);
    sendto(request.socket, "ACK", 4, 0, (struct sockaddr *)&client_addr, request.peer_length);
    std::string file_mod_time(buffer, static_cast<unsigned long>(received_bytes));
    filesystem::path path(out_path);
    logger_->debug("File modification time: {}", file_mod_time);

    std::fill(buffer, buffer + sizeof(buffer), 0);
    received_bytes = recvfrom(request.socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &request.peer_length);
    sendto(request.socket, "ACK", 4, 0, (struct sockaddr *)&client_addr, request.peer_length);
    std::string file_perm(buffer, static_cast<unsigned long>(received_bytes));

    filesystem::permissions(path, parse_file_permissions_from_string(file_perm));

    logger_->debug("Received permissions of file: {} ", file_perm);

    enable_socket_timeout(request);

    bool writable_packet;
    do {
        std::fill(buffer, buffer + sizeof(buffer), 0);
        received_bytes = recvfrom(request.socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &request.peer_length);
        if(received_bytes < 0) {
            throw std::runtime_error("Error receiving packet from client");
        }
        writable_packet = received_bytes > 1 || (received_bytes == 1 && buffer[0] != EOF_SYMBOL);
        if(writable_packet)
            output_file.write(buffer, received_bytes > BUFFER_SIZE? BUFFER_SIZE : received_bytes);
        logger_->debug("Received packet. Bytes: {}", received_bytes);
        sendto(request.socket, "ACK", 4, 0, (struct sockaddr *)&client_addr, request.peer_length);
    } while (writable_packet);
    output_file.close();
    filesystem::last_write_time(path, std::stol(file_mod_time));

    confirm_finish_handshake(request, client_addr);
    logger_->debug("Transferred file successfully!");

    disable_socket_timeout(request);
}

void File::send_list_files(dropbox_util::file_transfer_request request, const std::string& data) {
    struct sockaddr_in from{};

    struct timeval set_timeout_val = {0, TIMEOUT_US};
    if(setsockopt(request.socket, SOL_SOCKET, SO_RCVTIMEO, &set_timeout_val, sizeof(set_timeout_val)) < 0)
        logger_->error("Error setting timeout {}", set_timeout_val.tv_usec);

    int64_t packets = static_cast<int64_t>(std::ceil(data.size() / ((float) BUFFER_SIZE))),
            sent_packets = 0,
            received_bytes;
    char buffer[BUFFER_SIZE] {0};
    logger_->debug("Divided list_file transmission in {} packets.", packets);

    start_handshake(request, from);

    unsigned long data_position = 0;

    char ack[4];
    while(packets--) {
        std::fill(buffer, buffer + sizeof(buffer), 0);
        std::fill(ack, buffer + sizeof(ack), 0);
        std::string packet = data.substr(data_position, BUFFER_SIZE);
        data_position = packet.size();
        strncpy(buffer, packet.c_str(), data_position);
        logger_->debug("Sending buffer size of: {}", data_position);

        bool ack_error;
        int retransmissions = 0;
        do {
            sendto(request.socket, buffer, static_cast<size_t>(data_position), 0, (struct sockaddr *)&request.server_address,
                   request.peer_length);
            received_bytes = recvfrom(request.socket, ack, sizeof(ack), 0,(struct sockaddr *) &from, &request.peer_length);
            ack[3] = '\0';
            ack_error = received_bytes <= 0 || strcmp(ack, "ACK") != 0 != 0;
            if(ack_error) {
                logger_->error("Error receiving ACK, retransmitting packet of number {}", sent_packets);
                retransmissions++;
                if(retransmissions >= MAX_RETRANSMISSIONS) {
                    logger_->error("Achieved maximum retransmissions of {}, aborting file transmission", retransmissions);
                    throw std::runtime_error("Maximum retransmissions achieved");
                }
            }
            else
                sent_packets++;
        } while(ack_error);

        logger_->debug("ACK received");
    }

    logger_->debug("Sending end of file");
    buffer[0] = EOF_SYMBOL;
    sendto(request.socket, buffer, 1, 0, (struct sockaddr *)&request.server_address, request.peer_length);
    recvfrom(request.socket, ack, sizeof(ack), 0,(struct sockaddr *) &from, &request.peer_length);

    //finish handshake
    send_finish_handshake(request, from);
    logger_->debug("Successfully sent file_list");

    struct timeval unset_timeout_val = {0, 0};
    if(setsockopt(request.socket, SOL_SOCKET, SO_RCVTIMEO, &unset_timeout_val, sizeof(unset_timeout_val)) == 0)
        logger_->debug("Disabled packet timeout successfully");
    else
        logger_->debug("Error disabling packet timeout");
}

std::vector<std::vector<std::string>> File::receive_list_files(dropbox_util::file_transfer_request request) {
    struct sockaddr_in client_addr{0};
    establish_handshake(request, client_addr);

    char buffer[BUFFER_SIZE] = {0}, ack[4];
    int64_t received_bytes;

    struct timeval set_timeout_val = {0, TIMEOUT_US};
    if(setsockopt(request.socket, SOL_SOCKET, SO_RCVTIMEO, &set_timeout_val, sizeof(set_timeout_val)) < 0) {
        logger_->error("Error setting timeout {}", set_timeout_val.tv_usec);
    }

    bool writable_packet;
    std::stringstream received_data_stream;
    std::string received_data;

    do {
        std::fill(buffer, buffer + sizeof(buffer), 0);
        received_bytes = recvfrom(request.socket, buffer, sizeof(buffer), 0, (struct sockaddr *) &client_addr, &request.peer_length);
        if(received_bytes < 0) {
            // TODO Retry?
            throw std::runtime_error("Error receiving packet from client");
        }
        writable_packet = received_bytes > 1 || (received_bytes == 1 && buffer[0] != EOF_SYMBOL);
        if(writable_packet)
            received_data_stream << buffer;
        logger_->debug("Received packet. Bytes: {}", received_bytes);
        sendto(request.socket, "ACK", 4, 0, (struct sockaddr *)&client_addr, request.peer_length);
    } while (writable_packet);

    confirm_finish_handshake(request, client_addr);
    received_data = received_data_stream.str();
    logger_->debug("Transferred list of files successfully!");

    struct timeval unset_timeout_val = {0, 0};
    if(setsockopt(request.socket, SOL_SOCKET, SO_RCVTIMEO, &unset_timeout_val, sizeof(unset_timeout_val)) == 0)
        logger_->debug("Disabled packet timeout successfully");
    else
        logger_->debug("Error disabling packet timeout");

    return dropbox_util::parse_file_list_string(received_data);
}

void File::send_finish_handshake(file_transfer_request request, struct sockaddr_in &from) {
    char fin_ack[8];
    sendto(request.socket, "FIN", 4, 0, (struct sockaddr *)&request.server_address, request.peer_length);
    recvfrom(request.socket, fin_ack, sizeof(fin_ack), 0,(struct sockaddr *) &from, &request.peer_length);
    fin_ack[7] = '\0';
    if(strcmp(fin_ack,"FIN+ACK") != 0) {
        logger_->error("Error receiving FIN+ACK to close connection");
        throw std::runtime_error("Error finishing connection");
    }
    sendto(request.socket, "ACK", 4, 0, (struct sockaddr *)&request.server_address, request.peer_length);
}

void File::confirm_finish_handshake(file_transfer_request request, struct sockaddr_in &client_addr) {
    char fin[4], ack[4];
    recvfrom(request.socket, fin, sizeof(fin), 0, (struct sockaddr *) &client_addr, &request.peer_length);
    if(strcmp(fin,"FIN") != 0) {
        logger_->error("Error receiving FIN from client");
        throw std::runtime_error("Error finishing connection");
    }
    sendto(request.socket, "FIN+ACK", 8, 0, (struct sockaddr *)&client_addr, request.peer_length);
    recvfrom(request.socket, ack, sizeof(ack), 0, (struct sockaddr *) &client_addr, &request.peer_length);
    if(strcmp(ack,"ACK") != 0) {
        logger_->error("Error receiving ACK from client");
        throw std::runtime_error("Error finishing connection");
    }
}

void File::send_file_metadata(file_transfer_request request, struct sockaddr_in &from,
                                           filesystem::path &path) {
    struct stat st{};
    if(stat(request.in_file_path.c_str(), &st) != 0) {
        logger_->error("Error getting modification time of file {}", request.in_file_path);
        throw std::runtime_error("Error getting modification time of file");
    }
    logger_->debug("Modification time of file: {}", st.st_mtim.tv_sec );

    std::string mod_time = std::to_string(st.st_mtim.tv_sec);
    send_packet_with_retransmission(request, from, const_cast<char *>(mod_time.c_str()), mod_time.size());

    filesystem::perms file_permissions = filesystem::status(path).permissions();
    logger_->debug("File permissions:  {}", file_permissions);
    std::string file_perms_str = std::to_string(file_permissions);
    send_packet_with_retransmission(request, from, const_cast<char *>(file_perms_str.c_str()), file_perms_str.size());
}

void File::establish_handshake(file_transfer_request request, struct sockaddr_in &client_addr) {
    char ack[4], syn[4];
    recvfrom(request.socket, syn, sizeof(syn), 0, (struct sockaddr *) &client_addr, &request.peer_length);
    if(strcmp(syn,"SYN") != 0) {
        logger_->error("Error receiving SYN packet from client");
        throw std::runtime_error("Handshake error");
    }

    sendto(request.socket, "SYN+ACK", 8, 0, (struct sockaddr *)&client_addr, request.peer_length);
    recvfrom(request.socket, ack, sizeof(ack), 0, (struct sockaddr *) &client_addr, &request.peer_length);
    if(strcmp(ack,"ACK") != 0) {
        logger_->error("Error receiving ACK packet from client");
        throw std::runtime_error("Handshake error");
    }
}

void File::start_handshake(file_transfer_request request, struct sockaddr_in &from) {
    sendto(request.socket, "SYN", 4, 0, (struct sockaddr *)&request.server_address, request.peer_length);
    char syn_ack[8];
    recvfrom(request.socket, syn_ack, sizeof(syn_ack), 0, (struct sockaddr *) &from, &request.peer_length);
    if(strcmp(syn_ack, "SYN+ACK") != 0) {
        logger_->error("Error receiving SYN+ACK to open connection");
        throw std::runtime_error("Handshake failure");
    }
    sendto(request.socket, "ACK", 4, 0, (struct sockaddr *)&request.server_address, request.peer_length);
}


void File::disable_socket_timeout(const file_transfer_request &request) {
    struct timeval unset_timeout_val = {0, 0};
    if(setsockopt(request.socket, SOL_SOCKET, SO_RCVTIMEO, &unset_timeout_val, sizeof(unset_timeout_val)) == 0)
        logger_->debug("Disabled packet timeout successfully");
    else
        logger_->debug("Error disabling packet timeout");
}

void File::enable_socket_timeout(const file_transfer_request &request) {
    struct timeval set_timeout_val = {0, TIMEOUT_US};
    if(setsockopt(request.socket, SOL_SOCKET, SO_RCVTIMEO, &set_timeout_val, sizeof(set_timeout_val)) < 0)
        logger_->error("Error setting timeout {}", set_timeout_val.tv_usec);
}


filesystem::perms File::parse_file_permissions_from_string(const std::string &perms) {
    std::vector<filesystem::perms> perms_vec = {
            filesystem::owner_read,
            filesystem::owner_write,
            filesystem::owner_exe,
            filesystem::group_read,
            filesystem::group_write,
            filesystem::group_exe,
            filesystem::others_read,
            filesystem::others_write,
            filesystem::others_exe
    };
    auto perms_int = static_cast<uint16_t>(std::stoi(perms));

    perms_vec = map(perms_vec, [&](filesystem::perms p) -> filesystem::perms {
        return p & perms_int? p : filesystem::no_perms;
    });

    filesystem::perms p = filesystem::no_perms;
    for (auto &it : perms_vec) {
        p = p | it;
    }
    return p;
}
