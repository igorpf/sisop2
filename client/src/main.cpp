#include "../include/dropboxClient.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    try {
        Client client(123, "client123");
        client.login_server(util::LOOPBACK_IP, util::DEFAULT_SERVER_PORT);
        client.send_file("dropboxClient");
    } catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }
    return 0;
}
