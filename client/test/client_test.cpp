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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
