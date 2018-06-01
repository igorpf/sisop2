#include <iostream>
#include <exception>

#include <spdlog/spdlog.h>

#include "../include/dropboxClient.hpp"
#include "../include/sync_thread.hpp"
#include "../include/shell.hpp"
#include "../../util/include/LoggerFactory.hpp"

int main(int argc, char* argv[])
{
    auto logger = spdlog::stdout_color_mt("ClientMain");

    try {
        Client client;
        client.start_client(argc, argv);
        client.sync_client();

        SyncThread sync_thread;
        sync_thread.Start();

        Shell shell(client);
        shell.loop();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
