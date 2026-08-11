// ─────────────────────────────────────────────────────────────
// 📁 MirEngine/Core/Logging/Logger.hpp
// ─────────────────────────────────────────────────────────────
// 📝 УМНЫЙ ДНЕВНИК (ЛОГГЕР) ДЛЯ ВСЕЙ СИСТЕМЫ МИР 4D
//
// В большой программе, как на огромном заводе, происходит
// множество событий: деталь создалась, деталь сломалась,
// пользователь нажал кнопку. Чтобы инженеры понимали, что
// случилось, программа ведёт дневник — логгер.
//
// Логгер записывает сообщения с пометками:
//   🔵 Debug   — очень подробные заметки для разработчиков
//   🟢 Info    — обычные новости («Деталь создана»)
//   🟡 Warning — предупреждения («Кажется, тут ошибка»)
//   🔴 Error   — серьёзные ошибки («Не могу открыть файл!»)
//   💀 Critical — катастрофа, программа сейчас упадёт
//
// Можно выставить «уровень громкости» — например, показывать
// только Warning и Error, а тихие Debug и Info прятать.
//
// Все методы безопасны для вызова из нескольких потоков
// одновременно — как несколько человек, пишущих в один дневник,
// но аккуратно, не перебивая друг друга.
//
// Пример использования:
//   Logger log;
//   log.setLevel(LogLevel::Info);
//   log.info("Программа запущена");
//   log.warning("Батарейка садится");
//   log.error("Файл не найден");
//
// Чистый C++23, без внешних зависимостей.
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
    Debug,      // всё-всё, даже мельчайшие подробности
    Info,       // обычные рабочие сообщения
    Warning,    // что-то пошло не так, но программа жива
    Error,      // серьёзная ошибка, часть программы не работает
    Critical    // всё пропало, сейчас будем падать
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

// ─────────────────────────────────────────────────────────────
// 🧩 Удобные макросы (короткие команды)
// ─────────────────────────────────────────────────────────────
// Чтобы не писать каждый раз logger.debug(...), можно использовать
// короткие имена. Они автоматически подставят нужный текст.
// Например: LOG_INFO("Программа запущена");
// Превратится в: logger.info("Программа запущена");
//
// Эти макросы — как стикеры, которые ты клеишь в дневник.

#ifndef LOG_DEBUG
#define LOG_DEBUG(msg)    logger.debug(msg)
#define LOG_INFO(msg)     logger.info(msg)
#define LOG_WARNING(msg)  logger.warning(msg)
#define LOG_ERROR(msg)    logger.error(msg)
#define LOG_CRITICAL(msg) logger.critical(msg)
#endif

} // namespace mir