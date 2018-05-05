#include "../include/dropboxClient.hpp"

#include <iostream>

#include <spdlog/spdlog.h>


int main(int argc, char* argv[])
{
    try {
        Client client(123, "client123");
        client.login_server("127.0.0.1", 9001);
        client.send_file("dropboxClient");
    } catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }
    return 0;
}
