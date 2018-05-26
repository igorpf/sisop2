#include "../../util/include/dropboxUtil.hpp"
#include "../../util/include/string_formatter.hpp"
#include "../../util/include/File.hpp"

#include "../include/login_command_parser.hpp"
#include "../include/shell_command_parser.hpp"
#include "../include/dropboxClient.hpp"
#include "../include/shell.hpp"

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

/**
 * Classe que implementa a interface IClient para testar o shell
 */
class MockClient : public IClient {
public:
    ~MockClient() override = default;

    void sync_client() override {
        last_command_ = "sync_client";
    }

    void send_file(const std::string &filename) override {
        last_command_ = "send_file";
    }
    void get_file(const std::string &filename) override {
        last_command_ = "get_file";
    }
    void delete_file(const std::string &filename) override {
        last_command_ = "delete_file";
    }

    std::vector<std::vector<std::string>> list_server() override {
        last_command_ = "list_server";
        std::vector<std::vector<std::string>> temp;
        return temp;
    }
    std::vector<std::vector<std::string>> list_client() override {
        last_command_ = "list_client";
        std::vector<std::vector<std::string>> temp;
        return temp;
    }

    void close_session() override {
        active_session_ = false;
    };

    std::string last_command_;
    bool active_session_ = true;
};

/// LoginParserTest

TEST(LoginParserTest, ParseSpecifiedArguments)
{
    std::array<const char*, 4> argv = {"dropboxClient", "--userid=id1234", "--hostname=127.0.0.1", "--port=8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
}

TEST(LoginParserTest, ParsePositionalArguments)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
}

TEST(LoginParserTest, ParseIncompleteArguments)
{
    std::array<const char*, 3> argv = {"dropboxClient", "id1234", "127.0.0.1"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
}

TEST(LoginParserTest, ParseAndValidateHelpMessage)
{
    std::array<const char*, 2> argv = {"dropboxClient", "--help"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(argc, const_cast<char**>(argv.data())));
    ASSERT_NO_THROW(command_parser.ValidateInput());
}

TEST(LoginParserTest, ValidateWithoutParsing)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};

    LoginCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(LoginParserTest, ValidateCorrectInfo)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_NO_THROW(command_parser.ValidateInput());
}

TEST(LoginParserTest, ValidateShortUserid)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(LoginParserTest, ValidateInvalidHostname)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(LoginParserTest, ValidateInvalidPort)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "70000"};
    int argc = argv.size();

    LoginCommandParser command_parser;
    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(LoginParserTest, ShowHelpMessage)
{
    std::array<const char*, 2> argv = {"dropboxClient", "--help"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_TRUE(command_parser.ShowHelpMessage());
}

TEST(LoginParserTest, ShowHelpMessageWithoutParsing)
{
    std::array<const char*, 2> argv = {"dropboxClient", "--help"};

    LoginCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ShowHelpMessage());
}

TEST(LoginParserTest, DontShowHelpMessage)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_FALSE(command_parser.ShowHelpMessage());
}

TEST(LoginParserTest, GetUserid)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetUserid());
}

TEST(LoginParserTest, GetUseridWithoutParsing)
{
    LoginCommandParser command_parser;
    ASSERT_ANY_THROW(command_parser.GetUserid());
}

TEST(LoginParserTest, GetHostname)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetHostname());
}

TEST(LoginParserTest, GetHostnameWithoutParsing)
{
    LoginCommandParser command_parser;
    ASSERT_ANY_THROW(command_parser.GetHostname());
}

TEST(LoginParserTest, GetPort)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    LoginCommandParser command_parser;

    command_parser.ParseInput(argc, const_cast<char**>(argv.data()));
    command_parser.ValidateInput();

    ASSERT_NO_THROW(command_parser.GetPort());
}

TEST(LoginParserTest, GetPortWithoutParsing)
{
    LoginCommandParser command_parser;
    ASSERT_ANY_THROW(command_parser.GetPort());
}

/// ShellParserTest

TEST(ShellParserTest, NoArguments)
{
    std::vector<std::string> arguments = {};

    ShellCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(arguments));
}

TEST(ShellParserTest, TooManyArguments)
{
    std::vector<std::string> arguments = {"download", "remote_file", "hello"};

    ShellCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(arguments));
}

TEST(ShellParserTest, InvalidOperation)
{
    std::vector<std::string> arguments = {"hello"};

    ShellCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(arguments));
    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ShellParserTest, UploadWithExistentFile)
{
    std::string temp_file_name = StringFormatter() << "Testfile_" << util::get_random_number();
    TemporaryFile temp_file(temp_file_name);

    std::vector<std::string> arguments = {"upload", temp_file_name};

    ShellCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(arguments));
    ASSERT_NO_THROW(command_parser.ValidateInput());
    ASSERT_NO_THROW(command_parser.GetOperation());
    ASSERT_NO_THROW(command_parser.GetFilePath());
}

TEST(ShellParserTest, UploadWithInexistentFile)
{
    std::vector<std::string> arguments = {"upload", "inexistent_file"};

    ShellCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(arguments));
    ASSERT_ANY_THROW(command_parser.ValidateInput());
}

TEST(ShellParserTest, UploadNoPath)
{
    std::vector<std::string> arguments = {"upload"};

    ShellCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(arguments));
}

TEST(ShellParserTest, DownloadWithPath)
{
    std::vector<std::string> arguments = {"download", "remote_file"};

    ShellCommandParser command_parser;

    ASSERT_NO_THROW(command_parser.ParseInput(arguments));
    ASSERT_NO_THROW(command_parser.ValidateInput());
    ASSERT_NO_THROW(command_parser.GetOperation());
    ASSERT_NO_THROW(command_parser.GetFilePath());
}

TEST(ShellParserTest, DownloadNoPath)
{
    std::vector<std::string> arguments = {"download"};

    ShellCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ParseInput(arguments));
}

TEST(ShellParserTest, ValidOperationsNoPath)
{
    std::array<std::string, 5> available_operations = {"help", "list_server", "list_client", "get_sync_dir", "exit"};

    std::vector<std::string> arguments;
    ShellCommandParser command_parser;

    for (const auto& operation : available_operations) {
        arguments.emplace_back(operation);
        ASSERT_NO_THROW(command_parser.ParseInput(arguments));
        ASSERT_NO_THROW(command_parser.ValidateInput());
        ASSERT_NO_THROW(command_parser.GetOperation());
        ASSERT_ANY_THROW(command_parser.GetFilePath());
        arguments.clear();
    }
}

TEST(ShellParserTest, GetDataWithouParsing)
{
    ShellCommandParser command_parser;

    ASSERT_ANY_THROW(command_parser.ValidateInput());
    ASSERT_ANY_THROW(command_parser.GetOperation());
    ASSERT_ANY_THROW(command_parser.GetFilePath());
    ASSERT_ANY_THROW(command_parser.ShowHelpMessage());
}

/// ShellTest

TEST(ShellTest, ShowHelpMessage)
{
    std::stringstream local_stream;
    local_stream << "help\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    ASSERT_NO_THROW(shell.loop(local_stream));
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, SyncClient)
{
    std::stringstream local_stream;
    local_stream << "get_sync_dir\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("sync_client", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, Download)
{
    std::stringstream local_stream;
    local_stream << "download file\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("get_file", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, Upload)
{
    std::string temp_file_name = StringFormatter() << "Testfile_" << util::get_random_number();
    TemporaryFile temp_file(temp_file_name);

    std::stringstream local_stream;
    local_stream << "upload " << temp_file_name << "\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("send_file", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, Remove)
{
    std::stringstream local_stream;
    local_stream << "remove file\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("delete_file", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, ListServer)
{
    std::stringstream local_stream;
    local_stream << "list_server\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("list_server", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, ListClient)
{
    std::stringstream local_stream;
    local_stream << "list_client\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("list_client", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

TEST(ShellTest, MultipleCommands)
{
    std::stringstream local_stream;
    local_stream << "help\nsync_client\nlist_client\ndownload file\nexit";

    MockClient mock_client;
    Shell shell(mock_client);

    shell.loop(local_stream);

    ASSERT_EQ("get_file", mock_client.last_command_);
    ASSERT_FALSE(mock_client.active_session_);
}

/// ClientTest
// TODO(jfguimaraes) Testes para o cliente, list_client, load_from_disk

TEST(ClientTest, SendInexistentFile)
{
    std::array<const char*, 4> argv = {"dropboxClient", "id1234", "127.0.0.1", "8080"};
    int argc = argv.size();

    Client client;

    client.start_client(argc, const_cast<char**>(argv.data()));

    ASSERT_ANY_THROW(client.send_file("INEXISTENT_FILE"));
}

TEST(ClientTest, ServerOffline)
{
    std::string temp_file_name = StringFormatter() << "Testfile_" << util::get_random_number();
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
