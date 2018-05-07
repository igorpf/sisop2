#ifndef DROPBOX_FILE_HPP
#define DROPBOX_FILE_HPP

#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

namespace filesystem = boost::filesystem;

namespace DropboxUtil {
    class File {
    public:
        File();
        virtual ~File();

        void send_file(file_transfer_request request);
        void receive_file(file_transfer_request request);
        filesystem::perms parse_file_permissions_from_string(const std::string &perms);

    private:
        static const std::string LOGGER_NAME;
        std::shared_ptr<spdlog::logger> logger_;
    };
}

#endif //DROPBOX_FILE_HPP
