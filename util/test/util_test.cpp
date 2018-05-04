#include "../include/dropboxUtil.hpp"

#include <gtest/gtest.h>

TEST(FilePermissions, ReadOnlyFile)
{
    auto perms_int = static_cast<int16_t>(
        filesystem::owner_read | filesystem::group_read | filesystem::others_read
    );
    DropboxUtil::File file_util;
    filesystem::perms perms = file_util.parse_file_permissions_from_string(std::to_string(perms_int));
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
    DropboxUtil::File fileUtil;
    filesystem::perms perms = fileUtil.parse_file_permissions_from_string("509");
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
