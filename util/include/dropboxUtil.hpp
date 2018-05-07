#ifndef SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
#define SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H

#include <ctime>
#include <string>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

#include <netinet/in.h>
//TODO(jfguimaraes) Tornar Util uma biblioteca? Faria os includes ficarem mais limpos

namespace filesystem = boost::filesystem;

namespace DropboxUtil {

    // constants
    const int32_t BUFFER_SIZE = 64000; //approximately an ip packet size
    const int64_t TIMEOUT_US  = 50000; // to disable timeout, set 500000000000 as value
    const int8_t MAX_RETRANSMISSIONS = 20;
    const int8_t DEFAULT_ERROR_CODE = 1;
    const int8_t EOF_SYMBOL = -1;
    const int16_t DEFAULT_SERVER_PORT = 9001;
    const std::string LOOPBACK_IP("127.0.0.1");

    template <typename Collection, typename UnaryOperator>
    Collection map(Collection col, UnaryOperator op) {
        std::transform(col.begin(),col.end(),col.begin(),op);
        return col;
    }

    //structs definition
    typedef int SOCKET;
    typedef struct file_info {
        std::string name;
        int64_t size;
        time_t last_modification_time;
    } file_info;

    typedef struct file_transfer_request {
        int32_t port;
        struct sockaddr_in server_address;
        SOCKET socket;
        socklen_t peer_length;
        std::string ip;
        std::string in_file_path;
    } file_transfer_request;

    // utility functions
    std::vector<std::string> split_words_by_spaces(const std::string &phrase);
    std::string get_errno_with_message(const std::string &base_message = "");
}

#endif // SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
