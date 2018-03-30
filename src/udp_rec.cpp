
#include <iostream>
#include <stdlib.h>
#include <string.h>

#define SOCKET int
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv){
    struct  sockaddr_in peer;
    SOCKET  s;
    int porta, peerlen, rc, i;
    char buffer[100];

    if(argc < 2) {
        std::cout << "Utilizar:" << std::endl;
        std::cout << "rec -p <porta>" << std::endl;
        exit(1);
    }

    for(i=1; i < argc; i++) {
        if(argv[i][0]=='-'){
            switch(argv[i][1]){
                case 'p':
                    i++;
                    porta = atoi(argv[i]);
                    if(porta < 1024) {
                        std::cout << "Porta invalida" << std::endl;
                        exit(1);
                    }
                    break;
                default:
                    std::cout << "Parametro invalido " << i << ": " << argv[i] << std::endl;
                    exit(1);
            }
        }else{
            std::cout << "Parametro invalido " << i << ": " << argv[i] << std::endl;
            exit(1);
        }
    }

// Cria o socket na familia AF_INET (Internet) e do tipo UDP (SOCK_DGRAM)
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "Falha na criacao do socket" << std::endl;
        exit(1);
    }

// Define domínio, IP e porta a receber dados
    memset((void *) &peer,0,sizeof(struct sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_addr.s_addr = htonl(INADDR_ANY); // Recebe de qualquer IP
    peer.sin_port = htons(porta); // Recebe na porta especificada na linha de comando
    peerlen = sizeof(peer);

// Associa socket com estrutura peer
    if(bind(s,(struct sockaddr *) &peer, peerlen)) {
        std::cout << "Erro no bind" << std::endl;
        exit(1);
    }

    std::cout << "Socket inicializado. Aguardando mensagens..." << std::endl << std::endl;

// Recebe pacotes do cliente e responde com string "ACK"
    std::string ack("ack");
    while (1) {
        rc = recvfrom(s,buffer,sizeof(buffer),0,(struct sockaddr *) &peer,(socklen_t *)&peerlen);
        std::cout << "Recebido " << buffer << std::endl;

        sendto(s,ack.c_str(),ack.length(),0,(struct sockaddr *)&peer, peerlen);
        // printf("Enviado ACK\n\n");
    }

}