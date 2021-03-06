#include "../include/dropboxUtil.hpp"
#include "../include/string_formatter.hpp"

#include <sstream>
#include <cerrno>
#include <ctime>

#include <boost/algorithm/string.hpp>

using namespace dropbox_util;

std::once_flag rand_init;

std::vector<std::string> dropbox_util::split_words_by_token(const std::string &phrase, const std::string &token) {
    std::vector<std::string> words;
    boost::split(words, phrase, boost::is_any_of(token));
    return words;
}

std::string dropbox_util::get_errno_with_message(const std::string &base_message) {
    return StringFormatter() << base_message << ", error code " << errno;
}

int64_t dropbox_util::get_random_number() {
    std::call_once(rand_init, [](){std::srand(static_cast<unsigned int>(std::time(nullptr)));});
    return std::rand();
}

bool dropbox_util::starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool dropbox_util::ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

std::vector<std::vector<std::string>> dropbox_util::parse_file_list_string(const std::string &received_data) {
    std::vector<std::vector<std::string>> server_entries;
    unsigned long last_position = 0;
    unsigned long separator_position = 0;

    // Parse do header
    std::vector<std::string> header;

    separator_position = received_data.find(';', last_position);
    header.emplace_back(received_data.substr(last_position, separator_position - last_position));
    last_position = separator_position + 1;

    separator_position = received_data.find(';', last_position);
    header.emplace_back(received_data.substr(last_position, separator_position - last_position));
    last_position = separator_position + 1;

    separator_position = received_data.find('&', last_position);
    header.emplace_back(received_data.substr(last_position, separator_position - last_position));
    last_position = separator_position;

    if (last_position != std::string::npos)
        last_position++;

    server_entries.emplace_back(header);

    // Parse dos arquivos
    while (last_position != std::string::npos) {
        std::vector<std::string> info;
        std::string current_field;

        // Nome do arquivo
        separator_position = received_data.find(';', last_position);
        info.emplace_back(received_data.substr(last_position, separator_position - last_position));
        last_position = separator_position + 1;

        // Tamanho do arquivo
        separator_position = received_data.find(';', last_position);
        info.emplace_back(received_data.substr(last_position,separator_position - last_position));
        last_position = separator_position + 1;

        // Última modificação no arquivo
        separator_position = received_data.find('&', last_position);
        info.emplace_back(received_data.substr(last_position, separator_position - last_position));
        last_position = separator_position;

        if (last_position != std::string::npos)
            last_position++;

        // Adiciona na lista de arquivos
        server_entries.emplace_back(info);
    }

    return server_entries;
}

bool dropbox_util::should_ignore_file(const std::string &filename) {
    return starts_with(filename, ".") || ends_with(filename, "~");
}

std::string dropbox_util::get_error_from_message(const std::string &error_message) {
    auto found = error_message.find(ERROR_MESSAGE_INITIAL_TOKEN);
    if (found != std::string::npos) {
        return error_message.substr(found + ERROR_MESSAGE_INITIAL_TOKEN.size());
    }
    return error_message;
}

void dropbox_util::remove_filename_from_list(const std::string &filename,
                                             std::vector<dropbox_util::file_info> &file_list) {
    if (!file_list.empty())
        file_list.erase(std::remove_if(file_list.begin(), file_list.end(),
                                       [&filename] (const dropbox_util::file_info& info) ->
                                               bool {return filename == info.name;}),
                        file_list.end());
}

std::string dropbox_util::get_filename(const std::string &complete_file_path) {
    auto filename_index = complete_file_path.find_last_of('/');
    return filename_index != std::string::npos ? complete_file_path.substr(filename_index + 1) : complete_file_path;
}
