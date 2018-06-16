#ifndef DROPBOX_FILE_HPP
#define DROPBOX_FILE_HPP

#include <boost/filesystem.hpp>
#include <spdlog/spdlog.h>

#include "dropboxUtil.hpp"
#include "logger_wrapper.hpp"

namespace filesystem = boost::filesystem;

namespace dropbox_util {
    // TODO Acho que devia se chamar algo como TransferHelper
    class File {
    public:
        File();

        void send_file(file_transfer_request request);
        void receive_file(file_transfer_request request);

        void send_list_files(file_transfer_request request, const std::string& data);
        std::vector<std::vector<std::string>> receive_list_files(file_transfer_request request);
        filesystem::perms parse_file_permissions_from_string(const std::string &perms);

        static bool file_exists(const std::string &file_path);

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

        void send_packet_with_retransmission(file_transfer_request request, struct sockaddr_in from, char* packet, size_t packet_size);

        void enable_socket_timeout(const file_transfer_request &request);

        void disable_socket_timeout(const file_transfer_request &request);

        // Attributes
        static const std::string LOGGER_NAME;
        LoggerWrapper logger_;


    };
}

#endif //DROPBOX_FILE_HPP
