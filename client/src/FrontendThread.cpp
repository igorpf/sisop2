#include "../include/FrontendThread.hpp"
#include "../include/login_command_parser.hpp"

const std::string FrontendThread::LOGGER_NAME = "FrontendThread";

FrontendThread::FrontendThread(IClient &client_) : client_(client_),  logger_(LOGGER_NAME) {}

void FrontendThread::Run() {
    init_socket();

    while(true) {
        try {
            char buffer[dropbox_util::BUFFER_SIZE];
            recvfrom(socket_, buffer, sizeof(buffer), 0, (struct sockaddr *) &server_addr_, &peer_length_);

            auto tokens = dropbox_util::split_words_by_token(buffer, ";");
            auto ip = tokens[0];
            auto port = static_cast<int64_t>(std::stoi(tokens[1]));

            logger_->info("Received new address from server, ip {} port {}", ip, port);
            client_.change_primary_server_address(ip, port);
        } catch (const std::runtime_error &runtime_error) {
            logger_->error("Error getting new address from server {}", runtime_error.what());
            throw runtime_error;
        }
    }
}

void FrontendThread::init_socket() {
    if((socket_ = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
        logger_->error("Error creating socket");
        throw std::runtime_error("Error initializing FrontendThread socket");
    }

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(static_cast<uint16_t>(port_));
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    peer_length_ = sizeof(server_addr_);
}

void FrontendThread::parse_command_line_input(int argc, char **argv) {
    LoginCommandParser parser;
    parser.ParseInput(argc, argv);
    parser.ValidateInput();
    port_ = parser.GetFrontendPort();
}

