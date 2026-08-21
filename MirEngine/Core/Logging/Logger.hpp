
#pragma once

#include <string>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iostream>

namespace mir {

enum class LogLevel {
    Debug = 0,
    Info  = 1,
    Warning = 2,
    Error = 3,
    Critical = 4,
    Off = 6
};

class Logger {
public:

    Logger() : m_minLevel(LogLevel::Info) {}

    void setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_minLevel = level;
    }

    LogLevel getLevel() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_minLevel;
    }

    void debug(const std::string& message)   { log(LogLevel::Debug,    message); }
    void info(const std::string& message)    { log(LogLevel::Info,     message); }
    void warning(const std::string& message) { log(LogLevel::Warning,  message); }
    void error(const std::string& message)   { log(LogLevel::Error,    message); }
    void critical(const std::string& message){ log(LogLevel::Critical, message); }

    void log(LogLevel level, const std::string& message);

    static const char* levelToString(LogLevel level);

private:
    LogLevel m_minLevel;
    mutable std::mutex m_mutex;

    bool shouldLog(LogLevel level) const;
    void output(LogLevel level, const std::string& message);
};

inline Logger& globalLogger() {
    static Logger g_logger;
    return g_logger;
}

#ifndef MIR_LOG_SHORTCUTS
#define MIR_LOG_SHORTCUTS
#define LOG_DEBUG(msg)    ::mir::globalLogger().debug(msg)
#define LOG_INFO(msg)     ::mir::globalLogger().info(msg)
#define LOG_WARNING(msg)  ::mir::globalLogger().warning(msg)
#define LOG_ERROR(msg)    ::mir::globalLogger().error(msg)
#define LOG_CRITICAL(msg) ::mir::globalLogger().critical(msg)
#endif

}
