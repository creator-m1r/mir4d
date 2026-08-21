// MirEngine/Core/Logging/Logger.hpp
// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Core/Logging/Logger.hpp
// ─────────────────────────────────────────────────────────────
#pragma once

#include <string>
#include <mutex>           // для потокобезопасности (как замок на дневнике)
#include <chrono>          // чтобы ставить точное время записи
#include <sstream>         // для удобной сборки сложных сообщений
#include <iostream>        // для вывода на консоль (временно, потом можно в файл)

namespace mir {

// 🎚️ Уровни важности сообщений (от самого тихого до самого громкого)
enum class LogLevel {
    Debug = 0,   // всё-всё, даже мельчайшие подробности
    Info  = 1,   // обычные рабочие сообщения
    Warning = 2, // что-то пошло не так, но программа жива
    Error = 3,   // серьёзная ошибка, часть программы не работает
    Critical = 4,// всё пропало, программа сейчас упадёт
    Off = 6      // полностью отключено
};

// 📝 Сам умный дневник
class Logger {
public:
    // ── Конструктор ──────────────────────────────────────────
    // При создании дневника мы говорим ему, куда писать (пока на консоль).
    // Позже можно будет указать имя файла.
    Logger() : m_minLevel(LogLevel::Info) {}   // по умолчанию показываем Info и выше

    // ── Настройка «уровня громкости» ─────────────────────────
    void setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(m_mutex);   // запираем дневник на время изменения
        m_minLevel = level;
    }

    LogLevel getLevel() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_minLevel;
    }

    // ── Запись сообщений разного уровня ─────────────────────
    // Каждый метод — как ручка определённого цвета.

    void debug(const std::string& message)   { log(LogLevel::Debug,    message); }
    void info(const std::string& message)    { log(LogLevel::Info,     message); }
    void warning(const std::string& message) { log(LogLevel::Warning,  message); }
    void error(const std::string& message)   { log(LogLevel::Error,    message); }
    void critical(const std::string& message){ log(LogLevel::Critical, message); }

    // ── Самый гибкий метод: запись с указанием уровня ────────
    void log(LogLevel level, const std::string& message);

    // ── Помощник: превращает уровень в читаемую строку ───────
    static const char* levelToString(LogLevel level);

private:
    LogLevel m_minLevel;                // минимальный уровень, который мы показываем
    mutable std::mutex m_mutex;         // замок для безопасности (mutable чтобы работало в const методах)

    // Внутренняя работа: проверяет, нужно ли показывать сообщение,
    // и если да — выводит его с красивым оформлением.
    bool shouldLog(LogLevel level) const;
    void output(LogLevel level, const std::string& message);
};

// Простая глобальная точка доступа к логгеру для тех случаев, когда
// нужен быстрый однофайловый доступ к логированию (макросы ниже).
inline Logger& globalLogger() {
    static Logger g_logger;
    return g_logger;
}

// ─────────────────────────────────────────────────────────────
// 🧩 Удобные макросы (короткие команды)
// ─────────────────────────────────────────────────────────────
// Чтобы избежать конфликтов с внешними проектами, используем уникальный
// guard-имя MIR_LOG_SHORTCUTS. Макросы обращаются к mir::globalLogger().

#ifndef MIR_LOG_SHORTCUTS
#define MIR_LOG_SHORTCUTS
#define LOG_DEBUG(msg)    ::mir::globalLogger().debug(msg)
#define LOG_INFO(msg)     ::mir::globalLogger().info(msg)
#define LOG_WARNING(msg)  ::mir::globalLogger().warning(msg)
#define LOG_ERROR(msg)    ::mir::globalLogger().error(msg)
#define LOG_CRITICAL(msg) ::mir::globalLogger().critical(msg)
#endif

} // namespace mir
