#ifndef SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
#define SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H

#include <ctime>
#include <string>

typedef int SOCKET;
const uint16_t BUFFER_SIZE = 64000; //approximately an ip packet size
//#define TIMEOUT_US (500)  //valid timeout
const uint64_t TIMEOUT_US  = 5000000000; //invalid timeout
const uint8_t MAX_RETRANSMSSIONS = 20;
const uint8_t DEFAULT_ERROR_CODE = 1;

//TODO(jfguimaraes) Tornar Util uma biblioteca? Faria os includes ficarem mais limpos

typedef struct file_info {
    std::string name;
    uint64_t size;
    time_t last_modification_time;
} file_info;

typedef struct file_transfer_request {
    uint16_t port;
    uint16_t transfer_rate;
    std::string ip;
    std::string in_file_path;
    std::string out_file_path;
} file_transfer_request;

void send_file(file_transfer_request request);
void receive_file(file_transfer_request request);

typedef struct {
    file_transfer_request request;
    u_int16_t sequence_number;
    bool is_connected;
    bool has_finished;
} file_transfer_connection;

#endif // SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
