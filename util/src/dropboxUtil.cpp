#include "../include/dropboxUtil.hpp"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cstdlib>


#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


filesystem::perms parse_permissions_from_string(const std::string &perms)
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

void send_file(file_transfer_request request) {
    struct sockaddr_in server_address, from;
    int peer_length;
    SOCKET sock;
    char ack[4];

    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cerr << "Error creating socket" << std::endl ;
        exit(DEFAULT_ERROR_CODE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(request.port);
    server_address.sin_addr.s_addr = inet_addr(request.ip.c_str());
    peer_length = sizeof(server_address);

//    int sleepTime = (1000000.0/(request.transfer_rate/100000.0));
    std::ifstream input_file;
    input_file.open(request.in_file_path.c_str(), std::ios::binary);

    if(!input_file.is_open()) {
        std::cerr << "could not open file " << request.in_file_path << std::endl;
        exit(1);
    }

    struct timeval tv = {0, TIMEOUT_US};

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "Error setting timeout" << std::endl;
    }

    input_file.seekg(0, input_file.end);
    size_t file_length = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    int packets = ceil(file_length/((float) BUFFER_SIZE)), sent_packets = 0, received_bytes;
    char buffer[BUFFER_SIZE];
//    std::cout << "Divided in " << packets << " packets. File size: " << file_length <<  std::endl;

    //start handshake
    sendto(sock,"SYN",4,0,(struct sockaddr *)&server_address, peer_length);
    char syn_ack[8];
    recvfrom(sock,syn_ack,sizeof(syn_ack),0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(syn_ack, "SYN+ACK")) {
        std::cerr << "Error receiving SYN+ACK to open connection" << std::endl;
        exit(1);
    }
    sendto(sock,"ACK", 4,0,(struct sockaddr *)&server_address, peer_length);

    sendto(sock, request.in_file_path.c_str(), request.in_file_path.size(), 0,(struct sockaddr *)&server_address, peer_length);
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(ack, "ACK")) {
        std::cerr << "Error receiving ack of file path packet" << std::endl;
    }

    struct stat st;
    if(stat(request.in_file_path.c_str(), &st) != 0) {
        std::cerr << "Error getting modification time of file " << request.in_file_path << std::endl;
    }
    else
        std::cout << "Modification time of file: "<< st.st_mtim.tv_sec << std::endl;

    std::string mod_time = std::to_string(st.st_mtim.tv_sec);
    sendto(sock, mod_time.c_str(), mod_time.size(), 0,(struct sockaddr *)&server_address, peer_length);
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(ack, "ACK")) {
        std::cerr << "Error receiving ack of file modification time" << std::endl;
    }

    filesystem::path path(request.in_file_path);
    filesystem::perms file_permissions = filesystem::status(path).permissions();
    std::cout << "Permissions " << (file_permissions) << std::endl;

    std::string file_perms_str = std::to_string(file_permissions);
    sendto(sock, file_perms_str.c_str(), file_perms_str.size(), 0,(struct sockaddr *)&server_address, peer_length);
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    if(strcmp(ack, "ACK")) {
        std::cerr << "Error receiving ack of file permissions" << std::endl;
    }

    while(packets--) {
        memset(&buffer,0,sizeof(buffer));
        memset(&ack, 0 , sizeof(ack));
        input_file.read(buffer, sizeof(buffer));
        std::cout << "Sending buffer size of: " << input_file.gcount() << std::endl;
        bool ack_error;
        int retransmissions = 0;
        do {
            sendto(sock, buffer, input_file.gcount(), 0, (struct sockaddr *)&server_address, peer_length);
            received_bytes = recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
            ack[3] = '\0';
            ack_error = received_bytes <= 0 || strcmp(ack, "ACK") != 0;
            if(ack_error) {
                std::cerr << "Error receiving ACK, retransmitting packet of number " << sent_packets <<  std::endl;
                retransmissions++;
                if(retransmissions >= MAX_RETRANSMSSIONS) {
                    std::cerr << "Achieved maximum retransmissions of " << retransmissions
                              << ", aborting file transmission" <<  std::endl;
                    exit(DEFAULT_ERROR_CODE);
                }
            }
            else
                sent_packets++;
        } while(ack_error);

        std::cout << ack << " received" << std::endl;
//        usleep(sleepTime);
    }
    buffer[0] = EOF_SYMBOL;
    sendto(sock, buffer, 1, 0, (struct sockaddr *)&server_address, peer_length);
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    input_file.close();

    //finish handshake
    char fin_ack[8];
    sendto(sock, "FIN", 4, 0, (struct sockaddr *)&server_address, peer_length);
    recvfrom(sock, fin_ack, sizeof(fin_ack), 0,(struct sockaddr *) &from,(socklen_t *)&peer_length);
    fin_ack[7] = '\0';
    if(strcmp(fin_ack,"FIN+ACK")) {
        std::cerr << "Error receiving FIN+ACK to close connection" << std::endl;
        exit(1);
    }
    sendto(sock, "ACK", 4, 0, (struct sockaddr *)&server_address, peer_length);
}

void receive_file(file_transfer_request request) {
    struct sockaddr_in server_addr, client_addr;
    SOCKET sock;
    int peer_length, received_bytes;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Error creating socket" << std::endl;
        exit(1);
    }

    memset((void *) &server_addr,0,sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(request.port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    peer_length = sizeof(server_addr);

    if(bind(sock,(struct sockaddr *) &server_addr, peer_length)) {
        std::cerr << "Bind error" << std::endl;
        exit(1);
    }

    std::cout << "Initialized socket " << std::endl << std::endl;
    char buffer[BUFFER_SIZE+1];

    //start handshake
    char ack[4], syn[4];
    recvfrom(sock, syn, sizeof(syn), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(syn,"SYN"))
        exit(1);
    sendto(sock, "SYN+ACK", 8, 0, (struct sockaddr *)&client_addr, peer_length);
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(ack,"ACK"))
        exit(1);

    received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    sendto(sock, "ACK", 4,0,(struct sockaddr *)&client_addr, peer_length);
    std::string out_path(buffer, received_bytes);
    std::cout << "Receiving file of name: " << out_path << std::endl;
    std::ofstream output_file;
    output_file.open(out_path.c_str(), std::ios::binary);
    if(!output_file.is_open()) {
        std::cerr << "Could not receive file!" << std::endl;
        exit(1);
    }

    memset(&buffer,0,sizeof(buffer));
    received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    sendto(sock, "ACK", 4,0,(struct sockaddr *)&client_addr, peer_length);
    std::string file_mod_time(buffer, received_bytes);
    filesystem::path path(out_path);
    std::cout << "File modification time" << file_mod_time << std::endl;


    memset(&buffer,0,sizeof(buffer));
    received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    sendto(sock, "ACK", 4,0,(struct sockaddr *)&client_addr, peer_length);
    std::string file_perm(buffer, received_bytes);

    filesystem::permissions(path, parse_permissions_from_string(file_perm));

    std::cout << "Received permisions of file: " << file_perm << " " << parse_permissions_from_string(file_perm) << std::endl;

    struct timeval tv = {0, TIMEOUT_US};

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "Error setting timeout" << std::endl;
    }

    bool writable_packet;
    do {
        memset(&buffer,0,sizeof(buffer));
        received_bytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
        if(received_bytes < 0) {
            std::cerr << "error receiving packet from client" << std::endl;
            exit(1);
        }
        writable_packet = received_bytes > 1 || (received_bytes == 1 && buffer[0] != EOF_SYMBOL);
        if(writable_packet)
            output_file.write(buffer, received_bytes > BUFFER_SIZE? BUFFER_SIZE : received_bytes);
        std::cout << "Recebido mais um pacote, bytes recebidos: " << received_bytes << std::endl;
        sendto(sock, "ACK", 4,0,(struct sockaddr *)&client_addr, peer_length);
    } while (writable_packet);
    output_file.close();
    filesystem::last_write_time(path, std::stol(file_mod_time));

    char fin[4];
    recvfrom(sock,fin,sizeof(fin),0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(fin,"FIN")) {
        std::cerr << "error receiving FIN from client" << std::endl;
        exit(1);
    }
    sendto(sock, "FIN+ACK", 8, 0,(struct sockaddr *)&client_addr, peer_length);
    recvfrom(sock, ack, sizeof(ack),0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(ack,"ACK")) {
        std::cerr << "error receiving ACK from client" << std::endl;
        exit(1);
    }
}