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

void start_server()
{
    file_transfer_request request;
    request.ip = std::string("127.0.0.1");
    request.port = port;
    receive_file(request);
}
