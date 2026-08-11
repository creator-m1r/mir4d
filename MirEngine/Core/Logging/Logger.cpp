// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Core/Logging/Logger.cpp
// ─────────────────────────────────────────────────────────────
// ⚙️ Реализация умного дневника (логгера).
//
// Здесь мы наполняем жизнью те обещания, которые дали в Logger.hpp.
// Логгер умеет:
//   • Проверять, нужно ли показывать сообщение (фильтр по уровню).
//   • Выводить его в консоль (или позже в файл) с красивым форматом.
//   • Защищать себя от путаницы, когда несколько потоков пишут одновременно.
//
// Чистый C++23, без внешних зависимостей.
// ─────────────────────────────────────────────────────────────

#include "Logger.hpp"
#include <iostream>      // для std::cerr (стандартный поток ошибок)
#include <ctime>         // для работы со временем
#include <iomanip>       // чтобы красиво выровнять время (например, 02 вместо 2)
#include <sstream>       // чтобы собрать строку из кусочков

namespace mir {

// ─────────────────────────────────────────────────────────
// 🎨 Превращение уровня в цветную строку
// ─────────────────────────────────────────────────────────
const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARNING";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

// ─────────────────────────────────────────────────────────
// 🚦 Проверка: стоит ли показывать сообщение?
// ─────────────────────────────────────────────────────────
bool Logger::shouldLog(LogLevel level) const {
    // Сообщение будет показано, если его уровень НЕ НИЖЕ
    // (то есть такой же или более серьёзный), чем минимальный порог.
    return static_cast<int>(level) >= static_cast<int>(m_minLevel);
}

// ─────────────────────────────────────────────────────────
// 📤 Вывод сообщения в консоль
// ─────────────────────────────────────────────────────────
void Logger::output(LogLevel level, const std::string& message) {
    // Получаем текущее время (чтобы знать, когда запись сделана)
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    // localtime_s — безопасная версия для Windows, на других ОС используем localtime_r
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    // Собираем красивую строку: [УРОВЕНЬ] [ЧЧ:ММ:СС] сообщение
    std::stringstream ss;
    ss << "[" << levelToString(level) << "] "
       << "[" << std::setw(2) << std::setfill('0') << localTime.tm_hour << ":"
       << std::setw(2) << std::setfill('0') << localTime.tm_min  << ":"
       << std::setw(2) << std::setfill('0') << localTime.tm_sec  << "] "
       << message;

    // Выводим в консоль. Для ошибок и критических используем std::cerr (красный поток),
    // для остальных — std::cout (обычный поток).
    if (level >= LogLevel::Error) {
        std::cerr << ss.str() << std::endl;
    } else {
        std::cout << ss.str() << std::endl;
    }

    // В будущем здесь можно дописать сохранение в файл.
}

// ─────────────────────────────────────────────────────────
// 📝 Главный публичный метод log
// ─────────────────────────────────────────────────────────
void Logger::log(LogLevel level, const std::string& message) {
    // Захватываем замок, чтобы только один поток мог писать одновременно.
    std::lock_guard<std::mutex> lock(m_mutex);

    // Если сообщение недостаточно важное — игнорируем.
    if (!shouldLog(level)) {
        return;
    }

    // Иначе выводим его.
    output(level, message);
}

// ─────────────────────────────────────────────────────────
// 🚀 Вспомогательные короткие функции уже определены в .hpp,
// они просто вызывают log с нужным уровнем.
// ─────────────────────────────────────────────────────────

} // namespace mir