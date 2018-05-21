#include "../include/dropboxUtil.hpp"
#include "../include/string_formatter.hpp"
#include "../include/File.hpp"
#include "../include/table_printer.hpp"

#include <gtest/gtest.h>

/// File Permissions

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

/// String Formatter

TEST(StringFormatter, StringFormatter)
{
    std::string end_string;
    int64_t int_number = 2557;
    double double_number = 4829.87;
    std::string full_string = "this is a string";
    char one_char = 'a';

    end_string = StringFormatter() << one_char << int_number << full_string << double_number;

    ASSERT_EQ("a2557this is a string4829.87", end_string);
}

/// UtilityFunctions

TEST(UtilityFunctions, SplitOnSpaces)
{
    std::string original_string = "string with spaces";

    std::vector<std::string> words = DropboxUtil::split_words_by_token(original_string, " ");

    ASSERT_EQ("string", words[0]);
    ASSERT_EQ("with", words[1]);
    ASSERT_EQ("spaces", words[2]);
}

TEST(UtilityFunctions, ErrnoWithMessage)
{
    auto previous_errno = errno;
    errno = 2;
    std::string base_message = "Test message";
    std::string full_message = DropboxUtil::get_errno_with_message(base_message);

    EXPECT_EQ("Test message, error code 2", full_message);
    errno = previous_errno;
}

TEST(UtilityFunctions, RandomNumber)
{
    auto random_1 = DropboxUtil::get_random_number();
    auto random_2 = DropboxUtil::get_random_number();
    auto random_3 = DropboxUtil::get_random_number();

    ASSERT_NE(random_1, random_2);
    ASSERT_NE(random_1, random_3);
}

/// TablePrinter

TEST(TablePrinter, RegularTable)
{
    std::vector<std::vector<std::string>> table = {{"test1", "test2", "test3"}, {"hello_world", "s", "string"}};
    std::stringstream test_stream;

    TablePrinter table_printer(table);
    table_printer.Print(test_stream);

    std::string line1;
    std::string line2;

    std::getline(test_stream, line1);
    std::getline(test_stream, line2);

    ASSERT_EQ("test1       test2  test3  ", line1);
    ASSERT_EQ("hello_world s      string ", line2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
