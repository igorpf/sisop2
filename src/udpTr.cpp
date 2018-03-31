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
#define BUFFER_SIZE 64000 //aproximately an ip packet size

int main(int argc, char **argv){
    struct sockaddr_in peer;
    SOCKET s;
    double rate = 1.0;
    int porta, peerlen, rc, i;
    char ip[16];
    std::string filePath;

    if(argc < 9) {
        std::cout << "Utilizar: (argc " << argc << ")" << std::endl;
        std::cout << "trans -h <numero_ip> -p <porta> -r <rate> -f <arquivo>" << std::endl;
        exit(1);
    }

    // Pega parametros
    for(i = 1; i < argc; i++) {
        if(argv[i][0]=='-') {
            switch(argv[i][1]) {
                case 'h': // Numero IP
                    i++;
                    strcpy(ip, argv[i]);
                    break;
                case 'f': // Numero IP
                    i++;
                    filePath.assign(argv[i], sizeof(argv[i]));
//                    std::cout << filePath << std::endl;
                    break;
                case 'p': // porta
                    i++;
                    porta = atoi(argv[i]);
                    if(porta < 1024) {
                        std::cout << "Valor da porta invalido" << std::endl;
                        exit(1);
                    }
                    break;
                case 'r': // rate
                    i++;
                    rate = atoi(argv[i]) * 1000.0;
                    if(rate < 0) {
                        std::cout << "Valor de velocidade invalido" << std::endl;
                        exit(1);
                    }
                    break;
                default:
                    std::cout << "Parametro invalido " << i << ": " << argv[i] << std::endl;
                    exit(1);
            }
        } else {
            std::cout << "Parametro invalido " << i << ": " << argv[i] << std::endl;
            exit(1);
        }
    }

// Cria o socket na familia AF_INET (Internet) e do tipo UDP (SOCK_DGRAM)
    if((s = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cout << "Falha na criacao do socket" << std::endl ;
        exit(1);
    }

// Cria a estrutura com quem vai conversar
    peer.sin_family = AF_INET;
    peer.sin_port = htons(porta);
    peer.sin_addr.s_addr = inet_addr(ip);
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
    char buffer[BUFFER_SIZE+1];
    int packets = ceil(fileLength/((float) BUFFER_SIZE));
    std::cout << "Dividido em " << packets << " pacotes. File size: " << fileLength <<  std::endl;
    while(packets--) {
        memset(&buffer,0,sizeof(buffer));
        inputFile.read(buffer, sizeof(buffer));
        buffer[BUFFER_SIZE] = '\0';
//        std::cout << buffer << std::endl << " sz of " << sizeof(buffer) << std::endl;
        sendto(s, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer, peerlen);
        usleep(sleepTime);
        rc = recvfrom(s,ack,sizeof(ack),0,(struct sockaddr *) &peer,(socklen_t *)&peerlen);
        ack[3] = '\0';
        std::cout << ack << " veio pra ser ack, bits recebidos: " << rc << std::endl;
    }
    buffer[0] = '\0';
    sendto(s, buffer, 1, 0, (struct sockaddr *)&peer, peerlen);
    inputFile.close();



}