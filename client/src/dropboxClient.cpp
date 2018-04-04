#include "../../util/include/dropboxUtil.hpp"
#include "../include/dropboxClient.hpp"

#include <iostream>
#include <cstring>
#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void login_server(const std::string& host, int port)
{
    throw std::logic_error("Function not implemented");
}

void sync_client()
{
    throw std::logic_error("Function not implemented");
}

void send_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void get_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void delete_file(const std::string& filename)
{
    throw std::logic_error("Function not implemented");
}

void close_session()
{
    throw std::logic_error("Function not implemented");
}


const int port = 9001;
const std::string loopback_ip = "127.0.0.1";
void start_client() {
    std::cout << "Dropbox client running..." << std::endl;
    struct sockaddr_in peer;
    int peer_length;
    SOCKET sock;

    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cout << "Error creating socket" << std::endl ;
        exit(1);
    }

    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(loopback_ip.c_str());
    peer_length = sizeof(peer);

    char buffer[] = "Hello", end[]="end", serverResp[50];
    for(int i = 0; i < 10;i++) {
        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peer, peer_length);
        recvfrom(sock, serverResp, sizeof(serverResp), 0,(struct sockaddr *) &peer,(socklen_t *)&peer_length);
        std::cout << "Server responded: " << serverResp << std::endl;
    }
    sendto(sock, end, sizeof(end), 0, (struct sockaddr *)&peer, peer_length);
}
