#include "../include/dropboxUtil.hpp"

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * TODO: send all relevant file aspects:
 *  - name
 *  - modification date
 *  - permissions
 *  -
 *
 */

void send_file(file_transfer_request request) {
    struct sockaddr_in server_address, from;
    int peer_length;
    SOCKET sock;

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

    char ack[4];
    while(packets--) {
        memset(&buffer,0,sizeof(buffer));
        memset(&ack, 0 , sizeof(ack));
        input_file.read(buffer, sizeof(buffer));
//        std::cout << buffer << std::endl << " sz of " << sizeof(buffer) << std::endl;
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
    buffer[0] = '\0';
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
    std::string out_path = request.in_file_path;

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

    std::ofstream output_file;
    output_file.open(out_path.c_str(), std::ios::binary);
    if(!output_file.is_open()) {
        std::cerr << "Could not receive file!" << std::endl;
        exit(1);
    }

    //start handshake
    char ack[4], syn[4];
    recvfrom(sock, syn, sizeof(syn), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(syn,"SYN"))
        exit(1);
    sendto(sock, "SYN+ACK", 8, 0, (struct sockaddr *)&client_addr, peer_length);
    recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &client_addr,(socklen_t *)&peer_length);
    if(strcmp(ack,"ACK"))
        exit(1);

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
        writable_packet = received_bytes > 0 && buffer[0] != '\0';
        if(writable_packet) 
            output_file.write(buffer, received_bytes > BUFFER_SIZE? BUFFER_SIZE : received_bytes);
        std::cout << "Recebido mais um pacote, bytes recebidos: " << received_bytes << std::endl;
        sendto(sock, "ACK", 4,0,(struct sockaddr *)&client_addr, peer_length);
    } while (writable_packet);
    output_file.close();

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