#include "../include/login_command_parser.hpp"
#include "../include/dropboxClient.hpp"
#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/File.hpp"

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

TEST(ParserTest, ParseSpecifiedArguments)
{
    std::array<const char*, 4> argv = {"dropboxClient", "--userid=id1234", "--hostname=127.0.0.1", "--port=8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
}

TEST(ParserTest, ParsePositionalArguments)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
}

TEST(ParserTest, ParseIncompleteArguments)
{
    std::array<const char*, 3> argv = {"dropboxClient", "id1234", "127.0.0.1"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
}

TEST(ParserTest, ParseAndValidateHelpMessage)
{
    std::array<const char*, 2> argv = {"dropboxClient", "--help"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
    ASSERT_NO_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateWithoutParsing)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};

    LoginCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateCorrectInfo)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_NO_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateShortUserid)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateInvalidHostname)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ValidateInvalidPort)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "70000"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ParserTest, ShowHelpMessage)
{
    std::array<const char*, 2> argv = {"dropboxClient", "--help"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_TRUE(command_parser.ShowHelpMessage());
}

TEST(ParserTest, ShowHelpMessageWithoutParsing)
{
    std::array<const char*, 2> argv = {"dropboxClient", "--help"};

    LoginCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ShowHelpMessage());
}

TEST(ParserTest, DontShowHelpMessage)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_FALSE(command_parser.ShowHelpMessage());
}

TEST(ParserTest, GetUserid)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetUserid());
}

TEST(ParserTest, GetUseridWithoutParsing)
{
    LoginCommandParser command_parser;
    ASSERT_ANY_THROW(command_parser.GetUserid());
}

TEST(ParserTest, GetHostname)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetHostname());
}

TEST(ParserTest, GetHostnameWithoutParsing)
{
    LoginCommandParser command_parser;
    ASSERT_ANY_THROW(command_parser.GetHostname());
}

TEST(ParserTest, GetPort)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetPort());
}

TEST(ParserTest, GetPortWithoutParsing)
{
    LoginCommandParser command_parser;
    ASSERT_ANY_THROW(command_parser.GetPort());
}

TEST(Client, SendInexistentFile)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    Client client;

    client.start_client(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(client.send_file("INEXISTENT_FILE"));
}

TEST(Client, ServerOffline)
{
    std::string temp_file_name = "Testfile_" + std::to_string(util::get_random_number());
    TemporaryFile temp_file(temp_file_name);

    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    Client client;

    client.start_client(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(client.send_file(temp_file_name));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
