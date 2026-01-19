#ifndef BLADE_LOGGER_H
#define BLADE_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <memory>

namespace blade {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERR
};

class Logger {
public:
    static Logger& getInstance();

    void log(LogLevel level, const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void flush();

    void setLogFile(const std::string& filename);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
private:
    Logger();
    ~Logger();

    static std::string getLevelString(LogLevel level);
    static std::string getCurrentTimestamp();

    std::ofstream logFile_;
    std::mutex mutex_;
    std::string logFilePath_;
};

} // namespace blade

#endif // BLADE_LOGGER_H

