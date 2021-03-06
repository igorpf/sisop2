#ifndef SISOP2_CLIENT_INCLUDE_SYNC_THREAD_HPP
#define SISOP2_CLIENT_INCLUDE_SYNC_THREAD_HPP

#include <spdlog/spdlog.h>

#include "../../util/include/pthread_wrapper.hpp"

#include "iclient.hpp"
#include "../../util/include/logger_wrapper.hpp"

/**
 * Classe que encapsula a thread de sincronização do cliente
 */
class SyncThread : public PThreadWrapper {
public:
    explicit SyncThread(IClient& client);

protected:
    /**
     * Função que executa a sincronização em intervalos pré-definidos de sync_interval_in_microseconds_
     */
    void Run() override;

private:
    IClient& client_;
    int64_t sync_interval_in_microseconds_ = 50000;

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;
};

#endif //SISOP2_CLIENT_INCLUDE_SYNC_THREAD_HPP
