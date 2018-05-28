#include "../include/sync_thread.hpp"

#include <chrono>
#include <thread>
#include <iostream>

const std::string SyncThread::LOGGER_NAME = "SyncThread";

SyncThread::SyncThread()
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
    std::this_thread::sleep_for(std::chrono::seconds(5));
    logger_->debug("Sync running!\n");
}
