#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SOCKET int
#define BUFFER_SIZE 64000
/*
 * struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    read_timeout.tv_usec = 5*1000*1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
 * */


int main(int argc, char **argv){
    struct  sockaddr_in peer;
    SOCKET  s;
    int port, peerlen, rc;
    std::string outPath;

    if(argc < 5) {
        std::cout << "Use:" << std::endl;
        std::cout << "rec -p <port> -o <outFile>" << std::endl;
        exit(1);
    }

    for(int i=1; i < argc; i++) {
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'p':
                    i++;
                    port = atoi(argv[i]);
                    if(port < 1024) {
                        std::cout << "Invalid port" << std::endl;
                        exit(1);
                    }
                    break;
                case 'o':
                    i++;
                    outPath.assign(argv[i], sizeof(argv[i]));
                    break;
                default:
                    std::cout << "Invalid parameter " << i << ": " << argv[i] << std::endl;
                    exit(1);
            }
        } else {
            std::cout << "Invalid parameter " << i << ": " << argv[i] << std::endl;
            exit(1);
        }
    }

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "Error creating socket" << std::endl;
        exit(1);
    }

    memset((void *) &peer,0,sizeof(struct sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_addr.s_addr = htonl(INADDR_ANY); 
    peer.sin_port = htons(port);
    peerlen = sizeof(peer);

    if(bind(s,(struct sockaddr *) &peer, peerlen)) {
        std::cout << "Bind error" << std::endl;
        exit(1);
    }

    std::cout << "Initialized socket " << std::endl << std::endl;
    char buffer[BUFFER_SIZE];

    std::string ack("ack");
    std::ofstream outputFile;
    outputFile.open(outPath.c_str());
    if(!outputFile.is_open()) {
        std::cout << "Could not receive file!" << std::endl;
        exit(1);
    }

    do {
        memset(&buffer,0,sizeof(buffer));
//        std::cout << "Antes do receive: " << buffer << std::endl;
        rc = recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr *) &peer,(socklen_t *)&peerlen);
        outputFile << buffer << std::endl;
//        std::cout << "Recebido " << buffer << std::endl << std::endl << std::endl;
        std::cout << "Recebido mais um pacote" << std::endl << std::endl << std::endl;
        sendto(s,ack.c_str(),ack.length(),0,(struct sockaddr *)&peer, peerlen);
    } while (rc > 0 && buffer[0] != '\0');
    outputFile.close();


}