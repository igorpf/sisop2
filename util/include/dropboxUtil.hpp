#ifndef SISOP2_UTIL_INCLUDE_DROPBOXUTIL_HPP
#define SISOP2_UTIL_INCLUDE_DROPBOXUTIL_HPP

#include <ctime>
#include <string>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

#include <netinet/in.h>

//TODO(jfguimaraes) Tornar Util uma biblioteca

namespace filesystem = boost::filesystem;

namespace DropboxUtil {
    /// Constantes
    const int32_t BUFFER_SIZE = 64000; // approximately an ip packet size
    const int64_t TIMEOUT_US  = 50000; // to disable timeout, set 500000000000 as value
    const int8_t MAX_RETRANSMISSIONS = 20;
    const int8_t DEFAULT_ERROR_CODE = 1;
    const int8_t EOF_SYMBOL = -1;
    const int16_t DEFAULT_SERVER_PORT = 9001;
    const std::string LOOPBACK_IP = "127.0.0.1";
    const std::string COMMAND_SEPARATOR_TOKEN = ";";

    template <typename Collection, typename UnaryOperator>
    Collection map(Collection col, UnaryOperator op) {
        std::transform(col.begin(),col.end(),col.begin(),op);
        return col;
    }

    /// Definições de estruturas de dados
    typedef int SOCKET;

    struct file_info {
        std::string name;
        int64_t size;
        time_t last_modification_time;
    };

    struct file_transfer_request {
        struct sockaddr_in server_address;
        SOCKET socket;
        socklen_t peer_length;
        std::string in_file_path;
    };

    /// Funções de utilidade geral
    std::vector<std::string> split_words_by_spaces(const std::string &phrase);
    /// Utility functions
    std::vector<std::string> split_words_by_token(const std::string &phrase,
                                                  const std::string &token = COMMAND_SEPARATOR_TOKEN);
    std::string get_errno_with_message(const std::string &base_message = "");
    int64_t get_random_number();

    /// Parse de string de lista de arquivos
    std::vector<std::vector<std::string>> parse_file_list_string(const std::string& received_data);
}

#endif // SISOP2_UTIL_INCLUDE_DROPBOXUTIL_HPP
