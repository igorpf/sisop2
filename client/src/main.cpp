#include <iostream>
#include <exception>

#include "../include/dropboxClient.hpp"

int main(int argc, char* argv[])
{
    auto logger = spdlog::stdout_color_mt("ClientMain");

    try {
        Client client;
        client.start_client(argc, argv);
        client.send_file("dropboxClient");
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
