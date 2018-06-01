#include "../include/LoggerFactory.hpp"

std::shared_ptr<spdlog::logger> LoggerFactory::getLoggerForName(const std::string &loggerName) {
    auto logger = spdlog::get(loggerName)? spdlog::get(loggerName) : spdlog::basic_logger_mt(loggerName, "mainLogger.log");
    logger->set_level(spdlog::level::debug);
    return logger;
}
