#include <iostream>
#include <fstream>

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET  int
#define BUFFER_SIZE 64000 //approximately an ip packet size

int main(int argc, char **argv){
    struct sockaddr_in peer;
    SOCKET sock;
    double rate = 1.0;
    int port, peerlen;
    std::string ip;
    std::string filePath;

    if(argc < 9) {
        std::cout << "Use: (argc " << argc << ")" << std::endl;
        std::cout << "trans -h <ip_number> -p <port> -r <rate> -f <file>" << std::endl;
        exit(1);
    }

    for(int i = 1; i < argc; i++) {
        if(argv[i][0]=='-') {
            switch(argv[i][1]) {
                case 'h':
                    i++;
                    ip.assign(argv[i], strlen(argv[i]));
                    break;
                case 'f':
                    i++;
                    filePath.assign(argv[i], strlen(argv[i]));
                    break;
                case 'p':
                    i++;
                    port = atoi(argv[i]);
                    if(port < 1024) {
                        std::cout << "Invalid port" << std::endl;
                        exit(1);
                    }
                    break;
                case 'r': // rate
                    i++;
                    rate = atoi(argv[i]) * 1000.0;
                    if(rate < 0) {
                        std::cout << "Invalid rate parameter" << std::endl;
                        exit(1);
                    }
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

    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cout << "Error creating socket" << std::endl ;
        exit(1);
    }

    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(ip.c_str());
    peerlen = sizeof(peer);

    int sleepTime = (1000000.0/(rate/100000.0));
    char ack[4];
    std::ifstream inputFile;
    inputFile.open(filePath.c_str());

    if(!inputFile.is_open()) {
        std::cout << "could not open file " << filePath << std::endl;
        exit(1);
    }

    inputFile.seekg(0, inputFile.end);
    size_t fileLength = inputFile.tellg();
    inputFile.seekg(0, inputFile.beg);
    int packets = ceil(fileLength/((float) BUFFER_SIZE)), receivedBytes;

    char buffer[BUFFER_SIZE+1];
    std::cout << "Divided in " << packets << " packets. File size: " << fileLength <<  std::endl;

    while(packets--) {
        memset(&buffer,0,sizeof(buffer));
        inputFile.read(buffer, sizeof(buffer)-1);
        buffer[BUFFER_SIZE] = '\0';
//        std::cout << buffer << std::endl << " sz of " << sizeof(buffer) << std::endl;
        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer, peerlen);
        usleep(sleepTime);
        receivedBytes = recvfrom(sock, ack, sizeof(ack), 0,(struct sockaddr *) &peer,(socklen_t *)&peerlen);
        ack[3] = '\0';
        std::cout << ack << " received" << std::endl;
    }
    buffer[0] = '\0';
    sendto(sock, buffer, 1, 0, (struct sockaddr *)&peer, peerlen);
    inputFile.close();

}