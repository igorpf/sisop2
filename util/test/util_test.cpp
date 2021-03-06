#include <algorithm>

#include "../include/dropboxUtil.hpp"
#include "../include/string_formatter.hpp"
#include "../include/File.hpp"
#include "../include/table_printer.hpp"
#include "../include/LoggerFactory.hpp"

#include <gtest/gtest.h>

/// File Permissions

TEST(FilePermissions, ReadOnlyFile)
{
    auto perms_int = static_cast<int16_t>(
        filesystem::owner_read | filesystem::group_read | filesystem::others_read
    );
    dropbox_util::File file_util;
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
    dropbox_util::File file_util;
    filesystem::perms perms = file_util.parse_file_permissions_from_string("509");
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

/// Utility Functions

TEST(UtilityFunctions, SplitOnSpaces)
{
    std::string original_string = "string with spaces";

    std::vector<std::string> words = dropbox_util::split_words_by_token(original_string, " ");

    ASSERT_EQ("string", words[0]);
    ASSERT_EQ("with", words[1]);
    ASSERT_EQ("spaces", words[2]);
}

TEST(UtilityFunctions, ErrnoWithMessage)
{
    auto previous_errno = errno;
    errno = 2;
    std::string base_message = "Test message";
    std::string full_message = dropbox_util::get_errno_with_message(base_message);

    EXPECT_EQ("Test message, error code 2", full_message);
    errno = previous_errno;
}

TEST(UtilityFunctions, RandomNumber)
{
    auto random_1 = dropbox_util::get_random_number();
    auto random_2 = dropbox_util::get_random_number();
    auto random_3 = dropbox_util::get_random_number();

    ASSERT_NE(random_1, random_2);
    ASSERT_NE(random_1, random_3);
}

TEST(UtilityFunctions, StartsWith)
{
    std::string original_string = "Hello, world!";
    std::string valid_prefix = "Hello";
    std::string invalid_prefix = "helo";

    ASSERT_TRUE(dropbox_util::starts_with(original_string, valid_prefix));
    ASSERT_FALSE(dropbox_util::starts_with(original_string, invalid_prefix));
}

TEST(UtilityFunctions, EndsWith)
{
    std::string original_string = "Hello, world!";
    std::string valid_suffix = "world!";
    std::string invalid_suffix = "word!";

    ASSERT_TRUE(dropbox_util::ends_with(original_string, valid_suffix));
    ASSERT_FALSE(dropbox_util::ends_with(original_string, invalid_suffix));
}

TEST(UtilityFunctions, ShouldIgnoreFile)
{
    std::string normal_file = "filename.txt";
    std::string hidden_file = ".filename.txt";
    std::string backup_file = "filename.txt~";

    EXPECT_FALSE(dropbox_util::should_ignore_file(normal_file));
    EXPECT_TRUE(dropbox_util::should_ignore_file(hidden_file));
    EXPECT_TRUE(dropbox_util::should_ignore_file(backup_file));
}

TEST(UtilityFunctions, GetFilenameValidFilepath)
{
    std::string file_path = "/home/user/filename.txt";
    ASSERT_EQ(dropbox_util::get_filename(file_path), "filename.txt");
}

TEST(UtilityFunctions, GetFilenameBadString)
{
    std::string file_path = "filename.txt";
    ASSERT_EQ(dropbox_util::get_filename(file_path), "filename.txt");
}


TEST(UtilityFunctions, RemoveFilenameFromList)
{
    dropbox_util::file_info file_1 {"file_1", 10, 15623};
    dropbox_util::file_info file_2 {"file_2", 467, 187295};
    dropbox_util::file_info file_3 {"file_3", 9575, 126384};

    std::vector<dropbox_util::file_info> file_list;

    // Try to remove item from empty list
    EXPECT_NO_THROW(dropbox_util::remove_filename_from_list("file_1", file_list));

    // Add items to list
    file_list.emplace_back(file_1);
    file_list.emplace_back(file_2);
    file_list.emplace_back(file_3);

    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_1), file_list.end());
    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_2), file_list.end());
    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_3), file_list.end());

    // Remove existent file
    dropbox_util::remove_filename_from_list("file_2", file_list);

    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_1), file_list.end());
    EXPECT_EQ(std::find(file_list.begin(), file_list.end(), file_2), file_list.end());
    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_3), file_list.end());

    // Remove non-existent file
    dropbox_util::remove_filename_from_list("file_4", file_list);

    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_1), file_list.end());
    EXPECT_EQ(std::find(file_list.begin(), file_list.end(), file_2), file_list.end());
    EXPECT_NE(std::find(file_list.begin(), file_list.end(), file_3), file_list.end());
}

/// File list parsing

TEST(FileListParsing, FileListParsing)
{
    std::vector<std::vector<std::string>> expected_list = {{"name", "size", "modification_time"},
            {"name1", "15000", "1526852765"}, {"name2", "1456", "1526845650"}};

    std::string data {"name;size;modification_time&name1;15000;1526852765&name2;1456;1526845650"};

    std::vector<std::vector<std::string>> parsed_list = dropbox_util::parse_file_list_string(data);

    ASSERT_EQ(parsed_list, expected_list);
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

/// LoggerFactory

TEST(LoggerFactory, GivenLoggersWithSameNameShouldNotCrash)
{
    auto loggerName = "someLogger";
    std::shared_ptr<spdlog::logger> firstLogger, sameNameLogger;
    
    ASSERT_NO_THROW(firstLogger = LoggerFactory::getLoggerForName(loggerName));
    ASSERT_NO_THROW(sameNameLogger = LoggerFactory::getLoggerForName(loggerName));
}

// File
TEST(File, SendInexistentFile)
{
    dropbox_util::file_transfer_request request;
    request.in_file_path = "InexistentFile";

    dropbox_util::File file_util;
    ASSERT_ANY_THROW(file_util.send_file(request));
}

// Error messages
TEST(ErrorMessage, GetErrorFromErrorMessage)
{
    std::string error = " some error that occured",
            complete_error_message = dropbox_util::ERROR_MESSAGE_INITIAL_TOKEN + error;
    ASSERT_EQ(dropbox_util::get_error_from_message(complete_error_message), error);
}

// File
TEST(ErrorMessage, GetErrorFromNonErrorMessage)
{
    std::string error = "Some random error that occured";
    ASSERT_EQ(dropbox_util::get_error_from_message(error), error);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
