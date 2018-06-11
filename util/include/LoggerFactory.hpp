#ifndef DROPBOX_LOGGERFACTORY_HPP
#define DROPBOX_LOGGERFACTORY_HPP

#include <spdlog/spdlog.h>

class LoggerFactory {
public:
    /**
     * Builds a named logger. Must be destructed on class destructor
     * @param loggerName a unique string to identify to logger
     * @return a logger with the given loggerName
     */
    static std::shared_ptr<spdlog::logger> getLoggerForName(const std::string &loggerName, bool is_stdout = false, bool debug_enabled = false);
private:
    static size_t MAX_LOG_FILE_SIZE;
    static pthread_mutex_t logger_mutex_;
};

#endif //DROPBOX_LOGGERFACTORY_HPP
