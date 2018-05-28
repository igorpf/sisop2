#ifndef SISOP2_CLIENT_INCLUDE_SYNC_THREAD_HPP
#define SISOP2_CLIENT_INCLUDE_SYNC_THREAD_HPP

#include <spdlog/spdlog.h>

#include "../../util/include/pthread_wrapper.hpp"

class SyncThread : public PThreadWrapper {
public:
    SyncThread();
    ~SyncThread() override;

protected:
    void Run() override;

private:
    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;
};

#endif //SISOP2_CLIENT_INCLUDE_SYNC_THREAD_HPP
