#ifndef SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
#define SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H

#include <ctime>
#include <string>

//TODO(jfguimaraes) Tornar Util uma biblioteca? Faria os includes ficarem mais limpos

typedef struct file_info {
    std::string name;
    uint64_t size;
    time_t last_modification_time;
} file_info;

typedef int SOCKET;
const int8_t DEFAULT_ERROR_CODE = -1;
const uint16_t BUFFER_SIZE = 64000;

#endif // SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
