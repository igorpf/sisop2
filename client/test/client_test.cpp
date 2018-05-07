#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxClient.hpp"

#include <gtest/gtest.h>

namespace util = DropboxUtil;

TEST(Client, InvalidPort)
{
    util::file_transfer_request request{};
    request.in_file_path = "client_test";
    request.port = -1;
    request.ip = "127.0.0.1";
    util::File file_util;
    ASSERT_ANY_THROW(file_util.send_file(request));
}

TEST(Client, SendInexistentFile)
{
    util::file_transfer_request request{};
    request.in_file_path = "NON_EXISTING_FILE";
    request.port = 9000;
    request.ip = "127.0.0.1";
    util::File file_util;
    ASSERT_ANY_THROW(file_util.send_file(request));
}

TEST(Client, ServerNotOnline)
{
    util::file_transfer_request request{};
    request.in_file_path = "client_test";
    request.port = 9000;
    request.ip = "127.0.0.1";
    util::File file_util;
    ASSERT_ANY_THROW(file_util.send_file(request));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
