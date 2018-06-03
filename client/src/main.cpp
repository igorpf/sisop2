#include <iostream>
#include <exception>

#include <spdlog/spdlog.h>

#include "../include/dropboxClient.hpp"
#include "../include/file_watcher.hpp"
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

        FileWatcher file_watcher(client);
        file_watcher.Start();

        SyncThread sync_thread(client);
        sync_thread.Start();

        Shell shell(client);
        shell.loop();

        file_watcher.Join();
        sync_thread.Join();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
