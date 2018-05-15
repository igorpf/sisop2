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
    static std::shared_ptr<spdlog::logger> getLoggerForName(const std::string &loggerName);
};

#endif //DROPBOX_LOGGERFACTORY_HPP
