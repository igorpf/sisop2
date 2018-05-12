#include "../include/dropboxUtil.hpp"

#include <sstream>

std::once_flag rand_init;

std::vector<std::string> DropboxUtil::split_words_by_spaces(const std::string &phrase) {
    std::stringstream stream(phrase);
    std::string buffer;
    std::vector<std::string> words;
    while(stream >> buffer)
        words.push_back(buffer);
    return words;
}

std::string DropboxUtil::get_errno_with_message(const std::string &base_message) {
    std::ostringstream str_stream;
    str_stream << base_message << ", error code " << errno << std::endl;
    return str_stream.str();
}

int64_t DropboxUtil::get_random_number() {
    std::call_once(rand_init, [](){std::srand(static_cast<unsigned int>(std::time(nullptr)));});
    return std::rand();
}
