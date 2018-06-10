#include "../include/LoggerFactory.hpp"

size_t LoggerFactory::MAX_LOG_FILE_SIZE = 1 * 1024 * 1024; // 1MB

std::shared_ptr<spdlog::logger> LoggerFactory::getLoggerForName(const std::string &loggerName, bool debug_enabled) {
    auto logger = spdlog::get(loggerName)? spdlog::get(loggerName) : spdlog::rotating_logger_mt(loggerName, "mainLogger.log", MAX_LOG_FILE_SIZE, 1);
    if (debug_enabled) {
        logger->set_level(spdlog::level::debug);
    }
    logger->flush_on(debug_enabled? spdlog::level::debug : spdlog::level::info);
    return logger;
}
