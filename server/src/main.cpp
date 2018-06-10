#include "../include/dropboxServer.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    auto logger = LoggerFactory::getLoggerForName("ServerMain");
    try {
        // TODO Desenvolver um server_command_parser para obter a porta como parâmetro
        Server server;
        server.listen();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
