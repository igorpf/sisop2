#include "../include/dropboxClient.hpp"
#include "../../util/include/dropboxUtil.hpp"

#include <gtest/gtest.h>

TEST(Client, InvalidPort)
{
    file_transfer_request request{};
    request.in_file_path = "client_test";
    request.port = -1;
    request.ip = "127.0.0.1";
    ASSERT_ANY_THROW(send_file(request));
}

TEST(Client, SendInexistentFile)
{
    file_transfer_request request{};
    request.in_file_path = "NON_EXISTING_FILE";
    request.port = 9000;
    request.ip = "127.0.0.1";
    ASSERT_ANY_THROW(send_file(request));
}

TEST(Client, ServerNotOnline)
{
    file_transfer_request request{};
    request.in_file_path = "client_test";
    request.port = 9000;
    request.ip = "127.0.0.1";
    ASSERT_ANY_THROW(send_file(request));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
