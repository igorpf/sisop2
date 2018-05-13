#ifndef DROPBOX_FILE_HPP
#define DROPBOX_FILE_HPP

#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

#include "dropboxUtil.hpp"

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
        /**
         * Establishes an started handshake. Must be called by the part that is receiving the file
         */
        void establish_handshake(file_transfer_request request, struct sockaddr_in &client_addr);

        /**
         * Starts an TCP-like three way handshake. Must be called by the part that is sending the file
         */
        void start_handshake(file_transfer_request request, struct sockaddr_in &from);

        /**
         * Sends file related metadata like modification time and permissions
         */
        void send_file_metadata(file_transfer_request request, struct sockaddr_in &from, filesystem::path &path);

        /**
         * Signalizes end of file transmission starting the end handshake
         */
        void send_finish_handshake(file_transfer_request request, struct sockaddr_in &from);

        /**
         * Confirms the finish of the file transmission
         */
        void confirm_finish_handshake(file_transfer_request request, struct sockaddr_in &client_addr);

        static const std::string LOGGER_NAME;
        std::shared_ptr<spdlog::logger> logger_;
    };
}

#endif //DROPBOX_FILE_HPP
