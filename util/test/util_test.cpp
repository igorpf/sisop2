#include "../include/dropboxUtil.hpp"

#include <gtest/gtest.h>

TEST(FileTransfer, ReceiveAndSend)
{
//    file_transfer_request request;
//    request.ip = "192.168.0.11";
//    request.ip = "127.0.0.1";
//    request.transfer_rate = 1000;
//    request.port = 9000;
//    request.in_file_path = "files/in_ascii.txt";
//    request.out_file_path = "files/out_ascii.txt";
//    receive_file(request);
//    send_file(request);
    ASSERT_TRUE(true);
}

TEST(FilePermissions, ReadOnlyFile)
{
    auto perms_int = static_cast<uint16_t>(
        filesystem::owner_read | filesystem::group_read | filesystem::others_read
    );
    filesystem::perms perms = parse_permissions_from_string(std::to_string(perms_int));
    ASSERT_TRUE(perms & filesystem::owner_read);
    ASSERT_FALSE(perms & filesystem::owner_write);
    ASSERT_FALSE(perms & filesystem::owner_exe);
    ASSERT_TRUE(perms & filesystem::group_read);
    ASSERT_FALSE(perms & filesystem::group_write);
    ASSERT_FALSE(perms & filesystem::group_exe);
    ASSERT_TRUE(perms & filesystem::others_read);
    ASSERT_FALSE(perms & filesystem::others_write);
    ASSERT_FALSE(perms & filesystem::others_exe);
}

TEST(FilePermissions, ExecutableFile)
{
    filesystem::perms perms = parse_permissions_from_string("509");
    ASSERT_TRUE(perms & filesystem::owner_read);
    ASSERT_TRUE(perms & filesystem::owner_write);
    ASSERT_TRUE(perms & filesystem::owner_exe);
    ASSERT_TRUE(perms & filesystem::group_read);
    ASSERT_TRUE(perms & filesystem::group_write);
    ASSERT_TRUE(perms & filesystem::group_exe);
    ASSERT_TRUE(perms & filesystem::others_read);
    ASSERT_FALSE(perms & filesystem::others_write);
    ASSERT_TRUE(perms & filesystem::others_exe);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
