#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <ctime>

namespace blade {

Logger::Logger() {
    // Create logs directory if it doesn't exist
    std::filesystem::create_directories("logs");

    // Generate log filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &time);

    std::ostringstream oss;
    oss << "logs/blade_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << ".log";

    logFilePath_ = oss.str();
    logFile_.open(logFilePath_, std::ios::app);

    if (logFile_.is_open()) {
        log(LogLevel::INFO, "=== BLADE Log Session Started ===");
    }
}

Logger::~Logger() {
    if (logFile_.is_open()) {
        log(LogLevel::INFO, "=== BLADE Log Session Ended ===");
        logFile_.close();
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (logFile_.is_open()) {
        logFile_ << "[" << getCurrentTimestamp() << "] "
                 << "[" << getLevelString(level) << "] "
                 << message << std::endl;
        logFile_.flush();
    }
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERR, message);
}

void Logger::setLogFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (logFile_.is_open()) {
        logFile_.close();
    }

    logFilePath_ = "logs/" + filename;
    logFile_.open(logFilePath_, std::ios::app);
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERR:     return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm{};
    localtime_s(&tm, &time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

} // namespace blade

