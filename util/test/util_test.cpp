#include "../include/dropboxUtil.hpp"

#include <gtest/gtest.h>

TEST(FileTransfer, ReceiveAndSend)
{
    file_transfer_request request;
//    request.ip = "192.168.0.11";
    request.ip = "127.0.0.1";
    request.transfer_rate = 1000;
    request.port = 9000;
    request.in_file_path = "files/in_ascii.txt";
    request.out_file_path = "files/out_ascii.txt";
    receive_file(request);
    send_file(request);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
