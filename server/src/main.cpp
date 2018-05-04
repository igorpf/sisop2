#include "../include/dropboxServer.hpp"

#include <spdlog/spdlog.h>

const auto logger = spdlog::stdout_color_mt("server");

int main(int argc, char* argv[])
{
    try {
        start_server();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }


    return 0;
}
