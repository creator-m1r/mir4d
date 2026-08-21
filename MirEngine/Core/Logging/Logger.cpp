
#include "Logger.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace mir {

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARNING";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        case LogLevel::Off:      return "OFF";
    }
    return "UNKNOWN";
}

bool Logger::shouldLog(LogLevel level) const {

    if (m_minLevel == LogLevel::Off) {
        return false;
    }

    return static_cast<int>(level) >= static_cast<int>(m_minLevel);
}

void Logger::output(LogLevel level, const std::string& message) {

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;

#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::stringstream ss;
    ss << "[" << levelToString(level) << "] "
       << "[" << std::setw(2) << std::setfill('0') << localTime.tm_hour << ":"
       << std::setw(2) << std::setfill('0') << localTime.tm_min  << ":"
       << std::setw(2) << std::setfill('0') << localTime.tm_sec  << "] "
       << message;

    if (level == LogLevel::Error || level == LogLevel::Critical) {
        std::cerr << ss.str() << std::endl;
    } else if (level == LogLevel::Off) {

    } else {
        std::cout << ss.str() << std::endl;
    }

}

void Logger::log(LogLevel level, const std::string& message) {

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!shouldLog(level)) {
        return;
    }

    output(level, message);
}

}
