#include "../include/dropboxUtil.hpp"
#include "../include/string_formatter.hpp"

#include <sstream>
#include <cerrno>

#include <boost/algorithm/string.hpp>

std::once_flag rand_init;

std::vector<std::string> DropboxUtil::split_words_by_spaces(const std::string &phrase) {
    std::vector<std::string> words;
    boost::split(words, phrase, boost::is_any_of(" "));
    return words;
}

std::string DropboxUtil::get_errno_with_message(const std::string &base_message) {
    return StringFormatter() << base_message << ", error code " << errno;
}

int64_t DropboxUtil::get_random_number() {
    std::call_once(rand_init, [](){std::srand(static_cast<unsigned int>(std::time(nullptr)));});
    return std::rand();
}
