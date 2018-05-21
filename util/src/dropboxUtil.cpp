#include "../include/dropboxUtil.hpp"
#include "../include/string_formatter.hpp"

#include <sstream>
#include <cerrno>
#include <ctime>

#include <boost/algorithm/string.hpp>

std::once_flag rand_init;

std::vector<std::string> DropboxUtil::split_words_by_token(const std::string &phrase, const std::string &token) {
    std::vector<std::string> words;
    boost::split(words, phrase, boost::is_any_of(token));
    return words;
}

std::string DropboxUtil::get_errno_with_message(const std::string &base_message) {
    return StringFormatter() << base_message << ", error code " << errno;
}

int64_t DropboxUtil::get_random_number() {
    std::call_once(rand_init, [](){std::srand(static_cast<unsigned int>(std::time(nullptr)));});
    return std::rand();
}

std::vector<std::vector<std::string>> DropboxUtil::parse_file_list_string(const std::string &received_data) {
    std::vector<std::vector<std::string>> server_entries;
    unsigned long last_position = 0;
    unsigned long separator_position = 0;

    while (last_position != std::string::npos) {
        std::vector<std::string> info;

        // Nome do arquivo
        separator_position = received_data.find(';', last_position);
        info.emplace_back(received_data.substr(last_position, separator_position - last_position));
        last_position = separator_position + 1;

        // Tamanho do arquivo
        separator_position = received_data.find(';', last_position);
        info.emplace_back(received_data.substr(last_position, separator_position - last_position));
        last_position = separator_position + 1;

        // Última modificação no arquivo
        separator_position = received_data.find('&', last_position);
        std::string timestamp_data = received_data.substr(last_position, separator_position - last_position);
        if (timestamp_data != "modification_time") {
            std::time_t timestamp = std::stoi(timestamp_data);
            std::tm *ptm = std::localtime(&timestamp);
            char readable_timestamp[50];
            std::strftime(readable_timestamp, 50, "%Y-%m-%d %H:%M:%S", ptm);
            info.emplace_back(readable_timestamp);
        } else {
            info.emplace_back(timestamp_data);
        }
        last_position = separator_position;

        if (last_position != std::string::npos)
            last_position++;

        // Adiciona na lista de arquivos
        server_entries.emplace_back(info);
    }

    return server_entries;
}
