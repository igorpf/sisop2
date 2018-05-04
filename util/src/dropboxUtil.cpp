#include "../include/dropboxUtil.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


filesystem::perms DropboxUtil::File::parse_file_permissions_from_string(const std::string &perms)
{
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

void DropboxUtil::File::send_file(file_transfer_request request) {
    struct sockaddr_in server_address{}, from{};
    int peer_length;
    SOCKET sock;
    char ack[4];

    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        throw std::runtime_error("Error creating socket");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(request.port);
    server_address.sin_addr.s_addr = inet_addr(request.ip.c_str());
    peer_length = sizeof(server_address);

    std::ifstream input_file;
    input_file.open(request.in_file_path.c_str(), std::ios::binary);

    if(!input_file.is_open()) {
        logger->error("Could not open file {}", request.in_file_path);
        throw std::runtime_error("Error opening file");
    }

    struct timeval tv = {0, TIMEOUT_US};

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        logger->error("Error setting timeout");
    }

    filesystem::path path(request.in_file_path);
    if (!filesystem::exists(path))
        throw std::runtime_error("file doesn't exist");
    int64_t file_length = 0;
    if (filesystem::is_regular_file(path))
        file_length = filesystem::file_size(path);
    else
        throw std::runtime_error("path does not point to a file");

    int64_t packets = static_cast<int64_t>(std::ceil(file_length / ((float) BUFFER_SIZE))),
            sent_packets = 0,
            received_bytes;
    char buffer[BUFFER_SIZE];
    logger->info("Divided in file transmission in {} packets. File size: {}", packets, file_length);

    //start handshake
    sendto(sock, "SYN", 4, 0, (struct sockaddr *)&server_address, static_cast<socklen_t>(peer_length));
    char syn_ack[8];
    recvfrom(sock,syn_ack,sizeof(syn_ack),0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(syn_ack, "SYN+ACK") != 0) {
        logger->error("Error receiving SYN+ACK to open connection");
        throw std::runtime_error("Handshake failure");
    }
    sendto(sock, "ACK", 4, 0, (struct sockaddr *)&server_address, static_cast<socklen_t>(peer_length));

    // send file name
    sendto(sock, request.in_file_path.c_str(), request.in_file_path.size(), 0, (struct sockaddr *)&server_address,
           static_cast<socklen_t>(peer_length));
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(ack, "ACK") != 0) {
        logger->error("Error receiving ack of file path packet");
        throw std::runtime_error("Packet confirmation error");
    }

    struct stat st{};
    if(stat(request.in_file_path.c_str(), &st) != 0) {
        logger->error("Error getting modification time of file {}", request.in_file_path);
        throw std::runtime_error("Error getting modification time of file");
    }
    logger->info("Modification time of file: {}", st.st_mtim.tv_sec );

    // send file modification time
    std::string mod_time = std::to_string(st.st_mtim.tv_sec);
    sendto(sock, mod_time.c_str(), mod_time.size(), 0, (struct sockaddr *)&server_address,
           static_cast<socklen_t>(peer_length));
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(ack, "ACK") != 0) {
        logger->error("Error receiving ack of file modification time");
        throw std::runtime_error("Packet confirmation error");
    }

    // send file permissions
    filesystem::perms file_permissions = filesystem::status(path).permissions();
    logger->info("File permissions:  {}", file_permissions);
    std::string file_perms_str = std::to_string(file_permissions);
    sendto(sock, file_perms_str.c_str(), file_perms_str.size(), 0, (struct sockaddr *)&server_address,
           static_cast<socklen_t>(peer_length));
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(ack, "ACK") != 0) {
        logger->error("Error receiving ack of file permissions");
        throw std::runtime_error("Packet confirmation error");
    }

    while(packets--) {
        std::fill(buffer, buffer + sizeof(buffer), 0);
        std::fill(ack, buffer + sizeof(ack), 0);
        input_file.read(buffer, sizeof(buffer));
        logger->debug("Sending buffer size of: {}", input_file.gcount());

        bool ack_error;
        int retransmissions = 0;
        do {
            sendto(sock, buffer, static_cast<size_t>(input_file.gcount()), 0, (struct sockaddr *)&server_address,
                   static_cast<socklen_t>(peer_length));
            received_bytes = recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
            ack[3] = '\0';
            ack_error = received_bytes <= 0 || strcmp(ack, "ACK") != 0 != 0;
            if(ack_error) {
                logger->error("Error receiving ACK, retransmitting packet of number {}", sent_packets);
                retransmissions++;
                if(retransmissions >= MAX_RETRANSMSSIONS) {
                    logger->error("Achieved maximum retransmissions of {}, aborting file transmission", retransmissions);
                    throw std::runtime_error("Maximum retransmissions achieved");
                }
            }
            else
                sent_packets++;
        } while(ack_error);

        logger->debug("ACK received");
    }
    logger->debug("Sending end of file");
    buffer[0] = EOF_SYMBOL;
    sendto(sock, buffer, 1, 0, (struct sockaddr *)&server_address, static_cast<socklen_t>(peer_length));
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    input_file.close();

    //finish handshake
    char fin_ack[8];
    sendto(sock, "FIN", 4, 0, (struct sockaddr *)&server_address, static_cast<socklen_t>(peer_length));
    recvfrom(sock, fin_ack, sizeof(fin_ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    fin_ack[7] = '\0';
    if(strcmp(fin_ack,"FIN+ACK") != 0) {
        logger->error("Error receiving FIN+ACK to close connection");
        throw std::runtime_error("Error finishing connection");
    }
    sendto(sock, "ACK", 4, 0, (struct sockaddr *)&server_address, static_cast<socklen_t>(peer_length));

    logger->info("Successfully sent file");
}

void DropboxUtil::File::receive_file(file_transfer_request request) {
    struct sockaddr_in server_addr {0}, client_addr{0};
    SOCKET sock;
    int64_t peer_length, received_bytes;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        throw std::runtime_error("Error creating socket");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(request.port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    peer_length = sizeof(server_addr);

    if(bind(sock, (struct sockaddr *) &server_addr, static_cast<socklen_t>(peer_length))) {
        throw std::runtime_error("Bind error");
    }

    logger->info("Initialized socket");

    char buffer[BUFFER_SIZE+1];

    //start handshake
    char ack[4], syn[4];
    recvfrom(sock, syn, sizeof(syn), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(syn,"SYN") != 0) {
        logger->error("Error receiving SYN packet from client");
        throw std::runtime_error("Handshake error");
    }

    sendto(sock, "SYN+ACK", 8, 0, (struct sockaddr *)&client_addr, static_cast<socklen_t>(peer_length));
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(ack,"ACK") != 0) {
        logger->error("Error receiving ACK packet from client");
        throw std::runtime_error("Handshake error");
    }


    received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    sendto(sock, "ACK", 4, 0, (struct sockaddr *)&client_addr, static_cast<socklen_t>(peer_length));
    std::string out_path(buffer, static_cast<unsigned long>(received_bytes));
    logger->info("Receiving file of name: {}", out_path);
    std::ofstream output_file;
    output_file.open(out_path.c_str(), std::ios::binary);
    if(!output_file.is_open()) {
        throw std::runtime_error("Error opening file");
    }

    std::fill(buffer, buffer + sizeof(buffer), 0);
    received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    sendto(sock, "ACK", 4, 0, (struct sockaddr *)&client_addr, static_cast<socklen_t>(peer_length));
    std::string file_mod_time(buffer, static_cast<unsigned long>(received_bytes));
    filesystem::path path(out_path);
    logger->info("File modification time: {}", file_mod_time);

    std::fill(buffer, buffer + sizeof(buffer), 0);
    received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    sendto(sock, "ACK", 4, 0, (struct sockaddr *)&client_addr, static_cast<socklen_t>(peer_length));
    std::string file_perm(buffer, static_cast<unsigned long>(received_bytes));

    filesystem::permissions(path, parse_file_permissions_from_string(file_perm));

    logger->info("Received permissions of file: {} ", file_perm);

    struct timeval tv = {0, TIMEOUT_US};

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        logger->error("Error setting timeout");
    }

    bool writable_packet;
    do {
        std::fill(buffer, buffer + sizeof(buffer), 0);
        received_bytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
        if(received_bytes < 0) {
            throw std::runtime_error("Error receiving packet from client");
        }
        writable_packet = received_bytes > 1 || (received_bytes == 1 && buffer[0] != EOF_SYMBOL);
        if(writable_packet)
            output_file.write(buffer, received_bytes > BUFFER_SIZE? BUFFER_SIZE : received_bytes);
        logger->debug("Received packet. Bytes: {}", received_bytes);
        sendto(sock, "ACK", 4, 0, (struct sockaddr *)&client_addr, static_cast<socklen_t>(peer_length));
    } while (writable_packet);
    output_file.close();
    filesystem::last_write_time(path, std::stol(file_mod_time));

    char fin[4];
    recvfrom(sock,fin,sizeof(fin),0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(fin,"FIN") != 0) {
        logger->error("Error receiving FIN from client");
        throw std::runtime_error("Error finishing connection");
    }
    sendto(sock, "FIN+ACK", 8, 0, (struct sockaddr *)&client_addr, static_cast<socklen_t>(peer_length));
    recvfrom(sock, ack, sizeof(ack),0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(ack,"ACK") != 0) {
        logger->error("Error receiving ACK from client");
        throw std::runtime_error("Error finishing connection");
    }
    logger->info("Transferred file successfully!");
}

DropboxUtil::File::File() {
    /**
     *  to write to a file, use spdlog::basic_logger_mt("File", "logs/log.txt")
     *  to write to stdout, use spdlog::stdout_color_mt("File")
     */
    logger = spdlog::basic_logger_mt("File", "log.txt");
    spdlog::set_level(spdlog::level::debug);
}
