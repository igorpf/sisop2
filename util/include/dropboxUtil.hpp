#ifndef SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
#define SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H

#include <ctime>
#include <string>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>
//TODO(jfguimaraes) Tornar Util uma biblioteca? Faria os includes ficarem mais limpos

typedef int SOCKET;
const int32_t BUFFER_SIZE = 64000; //approximately an ip packet size
const int64_t TIMEOUT_US  = 50000; // to disable timeout, set 500000000000 as value
const int8_t MAX_RETRANSMSSIONS = 20;
const int8_t DEFAULT_ERROR_CODE = 1;
const int8_t EOF_SYMBOL = -1;

namespace filesystem = boost::filesystem;

typedef struct file_info {
    std::string name;
    int64_t size;
    time_t last_modification_time;
} file_info;

typedef struct file_transfer_request {
    int32_t port;
    int32_t transfer_rate;
    std::string ip;
    std::string in_file_path;
    std::string out_file_path;
} file_transfer_request;

typedef struct {
    file_transfer_request request;
    int32_t sequence_number;
    bool is_connected;
    bool has_finished;
} file_transfer_connection;

template <typename Collection,typename UnaryOperator>
Collection map(Collection col, UnaryOperator op) {
    std::transform(col.begin(),col.end(),col.begin(),op);
    return col;
}

namespace DropboxUtil {
    class File {
    public:
        void send_file(file_transfer_request request);
        void receive_file(file_transfer_request request);
        filesystem::perms parse_file_permissions_from_string(const std::string &perms);
        File();
    private:
        std::shared_ptr<spdlog::logger> logger_;
    };
}

#endif // SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
