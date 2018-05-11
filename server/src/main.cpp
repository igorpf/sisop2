#include "../include/dropboxServer.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    auto logger = spdlog::stdout_color_mt("ServerMain");
    try {
        Server server;
        server.listen();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
