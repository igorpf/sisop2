#ifndef SISOP2_CLIENT_INCLUDE_FILE_WATCHER_HPP
#define SISOP2_CLIENT_INCLUDE_FILE_WATCHER_HPP

#include <sys/inotify.h>
#include <spdlog/spdlog.h>

#include "../../util/include/pthread_wrapper.hpp"
#include "iclient.hpp"

/**
 * Classe que encapsula a thread de monitoramento de arquivos na pasta sync_dir
 */
class FileWatcher : public PThreadWrapper {
public:
    explicit FileWatcher(IClient& client);
    ~FileWatcher() override;

protected:
    /**
     * Implementação do loop de monitoramento de mudanças nos arquivos na pasta sync_dir
     */
    void Run() override;

private:
    IClient& client_;

    static const std::string LOGGER_NAME;
    std::shared_ptr<spdlog::logger> logger_;

    const size_t EVENT_SIZE = sizeof (struct inotify_event);
    const size_t EVENT_BUF_LEN = 1024 * (EVENT_SIZE + 16);
};

#endif //SISOP2_CLIENT_INCLUDE_FILE_WATCHER_HPP
