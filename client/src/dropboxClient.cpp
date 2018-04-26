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

void login_server(const std::string& host, int port) {
    struct sockaddr_in server_address, from;
    int peer_length;
    SOCKET sock;


    if((sock = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        std::cerr << "Error creating socket" << std::endl ;
        exit(DEFAULT_ERROR_CODE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host.c_str());
    peer_length = sizeof(server_address);
    sendto(sock,"ACK", 4,0,(struct sockaddr *)&server_address, peer_length);
//    char c[] = {-1};
//    sendto(sock, c, 1, 0,(struct sockaddr *)&server_address, peer_length);
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
