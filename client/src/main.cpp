#include "../include/dropboxClient.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    auto logger = spdlog::stdout_color_mt("ClientMain");
    try {
        Client client(123, "client123");
        client.login_server(util::LOOPBACK_IP, util::DEFAULT_SERVER_PORT);
        client.send_file("dropboxClient");
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }
    return 0;
}
