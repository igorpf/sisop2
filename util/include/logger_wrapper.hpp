#ifndef DROPBOX_LOGGER_WRAPPER_HPP
#define DROPBOX_LOGGER_WRAPPER_HPP

#include <spdlog/logger.h>
#include "LoggerFactory.hpp"

/**
 * RAII wrapper for Loggers.
 * Acquires a new logger in the constructor and releases it in the destructor
 */

class LoggerWrapper {
public:

    explicit LoggerWrapper(const std::string &logger_name, bool is_stdout = false) : logger_name_(logger_name) {
        logger_ = LoggerFactory::getLoggerForName(logger_name);
    }

    virtual ~LoggerWrapper() {
        spdlog::drop(logger_name_);
    }

    std::shared_ptr<spdlog::logger> operator->() const {
        return logger_;
    }

private:
    std::string logger_name_;
    std::shared_ptr<spdlog::logger> logger_;
};


#endif //DROPBOX_LOGGER_WRAPPER_HPP
