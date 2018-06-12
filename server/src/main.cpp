#include "../include/dropboxServer.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    auto logger = LoggerWrapper("ServerMain");
    try {
        Server server;
        server.start(argc, argv);
        server.listen();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
