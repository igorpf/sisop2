#include "../include/sync_thread.hpp"

#include <chrono>
#include <thread>

const std::string SyncThread::LOGGER_NAME = "SyncThread";

SyncThread::SyncThread(IClient& client) : client_(client)
{
    // TODO This logger should output to a file
    logger_ = spdlog::stdout_color_mt(LOGGER_NAME);
}

SyncThread::~SyncThread()
{
    spdlog::drop(LOGGER_NAME);
}

void SyncThread::Run()
{
    while (client_.logged_in_) {
        try {
            std::this_thread::sleep_for(std::chrono::seconds(sync_interval_in_seconds_));

            if (!client_.logged_in_)
                break;

            logger_->debug("Sync running\n");
            client_.sync_client();
        } catch (std::exception &exception) {
            logger_->error(exception.what());
        }
    }
}
