#include <iostream>
#include <exception>

#include <spdlog/spdlog.h>

#include "../include/dropboxClient.hpp"
#include "../include/shell.hpp"
#include "../../util/include/LoggerFactory.hpp"

int main(int argc, char* argv[])
{
    auto logger = LoggerFactory::getLoggerForName("ClientMain");

    try {
        Client client;
        client.start_client(argc, argv);
        client.sync_client();

        Shell shell(client);
        shell.loop();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
