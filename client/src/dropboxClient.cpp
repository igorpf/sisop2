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
void start_client()
{
    file_transfer_request request;
    request.in_file_path = std::string("dropboxClient");
    request.ip = std::string(loopback_ip);
    request.port = port;
    request.transfer_rate = 1000;
    send_file(request);
}
