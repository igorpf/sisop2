#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxClient.hpp"

#include <gtest/gtest.h>

namespace util = DropboxUtil;

TEST(Client, InvalidPort)
{
    Client client(1, "1");
    client.login_server(util::LOOPBACK_IP, 0);
    ASSERT_ANY_THROW(client.send_file("client_test"));
}

TEST(Client, SendInexistentFile)
{
    util::file_transfer_request request{};Client client(1, "1");
    client.login_server(util::LOOPBACK_IP, util::DEFAULT_SERVER_PORT);
    ASSERT_ANY_THROW(client.send_file("INEXISTENT_FILE"));
}

TEST(Client, InvalidServer)
{
    Client client(1, "1");
    client.login_server("not an ip", util::DEFAULT_SERVER_PORT);
    ASSERT_ANY_THROW(client.send_file("client_test"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
