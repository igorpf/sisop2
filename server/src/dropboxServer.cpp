#include "../../util/include/dropboxUtil.hpp"
#include "../include/dropboxServer.hpp"

#include <stdexcept>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void sync_server()
{
    throw std::logic_error("Function not implemented");
}

void receive_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void send_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}


const int port = 9001;

void start_server(){
    std::cout << "Starting Dropbox server running..." << std::endl;

    struct sockaddr_in peer;
    int peer_length;
    SOCKET sock;
    struct hostent *hostp;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "Error creating socket" << std::endl;
        exit(1);
    }

    memset((void *) &peer,0,sizeof(struct sockaddr_in));
    peer.sin_family = AF_INET;
    peer.sin_addr.s_addr = htonl(INADDR_ANY);
    peer.sin_port = htons(port);
    peer_length = sizeof(peer);

    if(bind(sock,(struct sockaddr *) &peer, peer_length)) {
        std::cout << "Dropbox server Bind error" << std::endl;
        exit(1);
    }

    std::cout << "Initialized Dropbox server socket " << std::endl << std::endl;
    std::cout << "Dropbox server running..." << std::endl;

    char buffer[50];
    int received_bytes;
    while(true) {
        received_bytes = recvfrom(sock, buffer, sizeof(buffer), 0,(struct sockaddr *) &peer,(socklen_t *)&peer_length);
        std::cout << "Received: " << buffer << std::endl;
        hostp = gethostbyaddr((const char *)&peer.sin_addr.s_addr,
                              sizeof(peer.sin_addr.s_addr), AF_INET);
        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer, peer_length);
        if(strcmp(buffer, "end") == 0)
            break;
    }

    std::cout << "Dropbox server finished executing" << std::endl;
}