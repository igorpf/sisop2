#include "../include/LoggerFactory.hpp"
#include "../include/lock_guard.hpp"

size_t LoggerFactory::MAX_LOG_FILE_SIZE = 1 * 1024 * 1024; // 1MB
pthread_mutex_t LoggerFactory::logger_mutex_ = PTHREAD_MUTEX_INITIALIZER;

std::shared_ptr<spdlog::logger> LoggerFactory::getLoggerForName(const std::string &loggerName, bool is_stdout, bool debug_enabled) {
    LockGuard lock(logger_mutex_);
    auto logger = spdlog::get(loggerName);
    if (logger)
        return logger;
    logger = is_stdout? spdlog::stdout_color_mt(loggerName) :  spdlog::rotating_logger_mt(loggerName, "mainLogger.log", MAX_LOG_FILE_SIZE, 1);
    if (debug_enabled) {
        logger->set_level(spdlog::level::debug);
    }
    logger->flush_on(debug_enabled? spdlog::level::debug : spdlog::level::info);
    return logger;
}
