#include "../include/command_parser.hpp"
#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/File.hpp"
#include "../include/dropboxClient.hpp"

#include <gtest/gtest.h>

namespace util = DropboxUtil;

/**
 * Cria um arquivo temporário no construtor e o remove no destrutor
 * Conteúdo do arquivo: nome do arquivo
 */
class TemporaryFile {
public:
    explicit TemporaryFile(const std::string& file_name) : file_name_(file_name) {
        std::ofstream temp_file(file_name_);
        temp_file << file_name_ << std::flush;
    }

    ~TemporaryFile() {
        std::remove(file_name_.c_str());
    }

private:
    std::string file_name_;
};

TEST(Client, InvalidPort)
{
    Client client(1, "1");
    client.login_server(util::LOOPBACK_IP, 0);
    ASSERT_ANY_THROW(client.send_file("client_test"));
}

TEST(Client, SendInexistentFile)
#include "../include/dropboxClient.hpp"

TEST(ParserTest, ParseSpecifiedArguments)
{
    char *argv[] = {"dropboxClient", "--userid=id1234", "--hostname=127.0.0.1", "--port=8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc,argv));
}

TEST(ParserTest, ParsePositionalArguments)
{
    util::file_transfer_request request{};
    Client client(1, "1");
    client.login_server(util::LOOPBACK_IP, util::DEFAULT_SERVER_PORT);
    ASSERT_ANY_THROW(client.send_file("INEXISTENT_FILE"));
}

TEST(Client, InvalidServer)
{
    Client client(1, "1");
    client.login_server("not an ip", util::DEFAULT_SERVER_PORT);
    ASSERT_ANY_THROW(client.send_file("client_test"));
}

TEST(Client, ServerOffline)
{
    std::string temp_file_name = "Testfile_" + std::to_string(util::get_random_number());
    TemporaryFile temp_file(temp_file_name);

    util::file_transfer_request request{};
    Client client(1, "1");
    client.login_server(util::LOOPBACK_IP, util::DEFAULT_SERVER_PORT);
    ASSERT_ANY_THROW(client.send_file(temp_file_name));
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc,argv));
}

TEST(ParserTest, ParseIncompleteArguments)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(argc,argv));
}

TEST(ParserTest, ParseAndValidateHelpMessage)
{
    char *argv[] = {"dropboxClient", "--help", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc,argv));
    ASSERT_NO_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateWithoutParsing)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateCorrectInfo)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;
    command_parser.ParseInput(argc,argv);

    ASSERT_NO_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateShortUserid)
{
    char *argv[] = {"dropboxClient", "id", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;
    command_parser.ParseInput(argc,argv);

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateInvalidHostname)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;
    command_parser.ParseInput(argc,argv);

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateInvalidPort)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "70000", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;
    command_parser.ParseInput(argc,argv);

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ShowHelpMessage)
{
    char *argv[] = {"dropboxClient", "--help", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    command_parser.ParseInput(argc,argv);
    command_parser.ValidateInput();

    ASSERT_TRUE(command_parser.ShowHelpMessage());
}

TEST(ParserTest, ShowHelpMessageWithoutParsing)
{
    char *argv[] = {"dropboxClient", "--help", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ShowHelpMessage());
}

TEST(ParserTest, DontShowHelpMessage)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    command_parser.ParseInput(argc,argv);
    command_parser.ValidateInput();

    ASSERT_FALSE(command_parser.ShowHelpMessage());
}

TEST(ParserTest, GetUserid)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    command_parser.ParseInput(argc,argv);
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetUserid());
}

TEST(ParserTest, GetUseridWithoutParsing)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.GetUserid());
}

TEST(ParserTest, GetHostname)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    command_parser.ParseInput(argc,argv);
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetHostname());
}

TEST(ParserTest, GetHostnameWithoutParsing)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.GetHostname());
}

TEST(ParserTest, GetPort)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    command_parser.ParseInput(argc,argv);
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetPort());
}

TEST(ParserTest, GetPortWithoutParsing)
{
    char *argv[] = {"dropboxClient", "id1234", "127.0.0.1", "8080", NULL};
    int argc = sizeof(argv) / sizeof(char*) - 1;

    CommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.GetPort());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
