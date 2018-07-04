#include <iostream>
#include <exception>

#include <spdlog/spdlog.h>

#include "../include/dropboxClient.hpp"
#include "../include/file_watcher.hpp"
#include "../include/sync_thread.hpp"
#include "../include/shell.hpp"
#include "../../util/include/LoggerFactory.hpp"
#include "../include/FrontendThread.hpp"

int main(int argc, char* argv[])
{
    auto logger = LoggerWrapper("ClientMain", true);

    try {
        Client client;
        client.start_client(argc, argv);
        client.sync_client();

        FileWatcher file_watcher(client);
        file_watcher.Start();

        SyncThread sync_thread(client);
        sync_thread.Start();

        FrontendThread frontend_thread(client);
        frontend_thread.parse_command_line_input(argc, argv);
        frontend_thread.Start();

        Shell shell(client);
        shell.loop();

        file_watcher.Join();
        sync_thread.Join();
        frontend_thread.Join();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }

    return 0;
}
