#include "../include/dropboxClient.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
    Client client(123, "client123");
    client.login_server("127.0.0.1", 9001);
    std::cout << "Dropbox client running..." << std::endl;
    return 0;
}
