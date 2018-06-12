#include "../include/dropboxServer.hpp"
#include "../include/server_login_parser.hpp"

#include <gtest/gtest.h>

TEST(ServerLoginParser, ValidArgumentsPrimary)
{
    std::array<const char*, 3> argv = {"dropboxServer", "primary", "9001"};
    int argc = argv.size();

    ServerLoginParser parser;
    ASSERT_NO_THROW(parser.ParseInput(argc, const_cast<char**>(argv.data())));
    ASSERT_NO_THROW(parser.ValidateInput());
}

TEST(ServerLoginParser, ValidArgumentsBackup)
{
    std::array<const char*, 3> argv = {"dropboxServer", "backup", "9001"};
    int argc = argv.size();

    ServerLoginParser parser;
    ASSERT_NO_THROW(parser.ParseInput(argc, const_cast<char**>(argv.data())));
    ASSERT_NO_THROW(parser.ValidateInput());
}

TEST(ServerLoginParser, InvalidTypeArgument)
{
    std::array<const char*, 3> argv = {"dropboxServer", "sometype", "9001"};
    int argc = argv.size();

    ServerLoginParser parser;
    parser.ParseInput(argc, const_cast<char**>(argv.data()));
    ASSERT_ANY_THROW(parser.ValidateInput());
}

TEST(ServerLoginParser, TestIsPrimary)
{
    std::array<const char*, 3> argv = {"dropboxServer", "primary", "9001"};
    int argc = argv.size();

    ServerLoginParser parser;
    parser.ParseInput(argc, const_cast<char**>(argv.data()));
    parser.ValidateInput();
    ASSERT_TRUE(parser.isPrimary());
}

TEST(ServerLoginParser, TestNotIsPrimary)
{
    std::array<const char*, 3> argv = {"dropboxServer", "backup", "9001"};
    int argc = argv.size();

    ServerLoginParser parser;
    parser.ParseInput(argc, const_cast<char**>(argv.data()));
    parser.ValidateInput();
    ASSERT_FALSE(parser.isPrimary());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
