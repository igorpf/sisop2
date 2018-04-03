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

void send_file(file_transfer_request request) {
    struct sockaddr_in peer;
    int peer_length;
    SOCKET sock;

    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cout << "Error creating socket" << std::endl ;
        exit(1);
    }

    peer.sin_family = AF_INET;
    peer.sin_port = htons(request.port);
    peer.sin_addr.s_addr = inet_addr(request.ip.c_str());
    peer_length = sizeof(peer);

    int sleepTime = (1000000.0/(request.transfer_rate/100000.0));
    char ack[4];
    std::ifstream input_file;
    input_file.open(request.in_file_path.c_str());

    if(!input_file.is_open()) {
        std::cout << "could not open file " << request.in_file_path << std::endl;
        exit(1);
    }

    input_file.seekg(0, input_file.end);
    size_t file_length = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    int packets = ceil(file_length/((float) BUFFER_SIZE)), received_bytes;
    char buffer[BUFFER_SIZE+1];
//    std::cout << "Divided in " << packets << " packets. File size: " << file_length <<  std::endl;

    while(packets--) {
        memset(&buffer,0,sizeof(buffer));
        input_file.read(buffer, sizeof(buffer)-1);
        buffer[BUFFER_SIZE] = '\0';
//        std::cout << buffer << std::endl << " sz of " << sizeof(buffer) << std::endl;
        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer, peer_length);
        usleep(sleepTime);
        received_bytes = recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &peer,(socklen_t *)&peer_length);
        ack[3] = '\0';
        std::cout << ack << " received" << std::endl;
    }
    buffer[0] = '\0';
    sendto(sock, buffer, 1, 0, (struct sockaddr *)&peer, peer_length);
    input_file.close();
}

void receive_file(file_transfer_request request) {
    struct sockaddr_in peer;
    SOCKET sock;
    int peerlen, received_bytes;
    std::string out_path;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "Error creating socket" << std::endl;
        exit(1);
    }

    memset((void *) &peer,0,sizeof(struct sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_port = htons(request.port);
    peer.sin_addr.s_addr = inet_addr(request.ip.c_str());
    peerlen = sizeof(peer);

    if(bind(sock,(struct sockaddr *) &peer, peerlen)) {
        std::cout << "Bind error" << std::endl;
        exit(1);
    }

    std::cout << "Initialized socket " << std::endl << std::endl;
    char buffer[BUFFER_SIZE+1];

    std::string ack("ack");
    std::ofstream output_file;
    output_file.open(out_path.c_str());
    if(!output_file.is_open()) {
        std::cout << "Could not receive file!" << std::endl;
        exit(1);
    }

    do {
        memset(&buffer,0,sizeof(buffer));
//        std::cout << "Antes do receive: " << buffer << std::endl;
        received_bytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *) &peer,(socklen_t *)&peerlen);
        buffer[BUFFER_SIZE+1] = '\0';
        output_file << buffer ;
//        std::cout << "Recebido " << buffer << std::endl << std::endl << std::endl;
        std::cout << "Recebido mais um pacote" << std::endl << std::endl << std::endl;
        sendto(sock,ack.c_str(),ack.length(),0,(struct sockaddr *)&peer, peerlen);
    } while (received_bytes > 0 && buffer[0] != '\0');
    output_file.close();
}