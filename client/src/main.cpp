#include "../include/dropboxClient.hpp"

#include <spdlog/spdlog.h>

const auto logger = spdlog::stdout_color_mt("client");

int main(int argc, char* argv[])
{
    try {
        Client client(123, "client123");
        client.login_server("127.0.0.1", 9001);
//        std::cout << "Dropbox client running..." << std::endl;
        start_client();
    } catch (std::exception &exception) {
        logger->error(exception.what());
    }
    return 0;
}
