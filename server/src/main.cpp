#include "../include/dropboxServer.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    try {
        Server server;
        server.listen();
    } catch (std::exception &exception) {
        std::cerr << exception.what() << std::endl;
    }

    return 0;
}
