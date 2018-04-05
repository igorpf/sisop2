#ifndef SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
#define SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H

#include <ctime>
#include <string>

//TODO(jfguimaraes) Tornar Util uma biblioteca? Faria os includes ficarem mais limpos

#define TRAVIS_TEST "This is only to trigger a Travis CI build"
#define TRAVIS_TEST_2 "This is also only to trigger a Travis CI build"

typedef struct file_info {
    std::string name;
    uint64_t size;
    time_t last_modification_time;
} file_info;

#endif // SISOP2_UTIL_INCLUDE_DROPBOXUTIL_H
