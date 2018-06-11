#ifndef SISOP2_CLIENT_INCLUDE_FILE_WATCHER_HPP
#define SISOP2_CLIENT_INCLUDE_FILE_WATCHER_HPP

#include <ctime>
#include <sys/inotify.h>
#include <spdlog/spdlog.h>

#include "../../util/include/pthread_wrapper.hpp"
#include "iclient.hpp"
#include "../../util/include/logger_wrapper.hpp"

/**
 * Classe que encapsula a thread de monitoramento de arquivos na pasta sync_dir
 */
class FileWatcher : public PThreadWrapper {
public:
    explicit FileWatcher(IClient& client);

protected:
    /**
     * Implementação do loop de monitoramento de mudanças nos arquivos na pasta sync_dir
     */
    void Run() override;

private:
    /**
     * Salva o registro de um arquivo modificado na lista de modificados do objeto cliente
     */
    void AddModifiedFile(const std::string& filename);

    IClient& client_;

    static const std::string LOGGER_NAME;
    LoggerWrapper logger_;

    const size_t EVENT_SIZE = sizeof (struct inotify_event);
    const size_t EVENT_BUF_LEN = 1024 * (EVENT_SIZE + 16);

    std::time_t event_timestamp_;
};

#endif //SISOP2_CLIENT_INCLUDE_FILE_WATCHER_HPP
