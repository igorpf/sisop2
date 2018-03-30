/**
* Retirado do template baixado na página dos laboratórios de redes
*
* Modificado por Christian Schmitz e Igor Pires Ferreira
*
* Testada a compilação em Linux
*/

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SOCKET  int

int main(int argc, char **argv){
    struct sockaddr_in peer;
    SOCKET s;
    double rate = 1.0;
    int porta, peerlen, rc, i;
    char ip[16];

    if(argc < 7) {
        std::cout << "Utilizar:" << std::endl;
        std::cout << "trans -h <numero_ip> -p <porta> -r <rate>" << std::endl;
        exit(1);
    }

    // Pega parametros
    for(i=1; i<argc; i++) {
        if(argv[i][0]=='-') {
            switch(argv[i][1]) {
                case 'h': // Numero IP
                    i++;
                    strcpy(ip, argv[i]);
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

    // Envia pacotes Hello e aguarda resposta
    //time to sleep in us
    int sleepTime = (1000000.0/(rate/100000.0));
    char ack[4];
    std::string buffer("Hello");
    while(1) {
        sendto(s, buffer.c_str(), buffer.length(), 0, (struct sockaddr *)&peer, peerlen);
        usleep(sleepTime);
        rc = recvfrom(s,ack,sizeof(ack),0,(struct sockaddr *) &peer,(socklen_t *)&peerlen);
        std::cout << ack << " veio pra ser ack, bits recebidos: " << rc << std::endl;
    }
}