#include "../include/dropboxServer.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "Dropbox server running..." << std::endl;
    Server server;
    server.start();
    server.listen();
    return 0;
}
