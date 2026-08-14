// MirEngine/Core/Engine/EngineConfig.hpp
// ⚙️ Настройки движка MirEngine — конфигурация, передаваемая при создании Engine.
//
// EngineConfig позволяет настроить поведение движка без изменения исходного кода.
// Передаётся в конструктор Engine. Все поля имеют разумные значения по умолчанию,
// поэтому EngineConfig{} уже готов к использованию «из коробки».
//
// Поля:
//   • enableLogging     — писать ли сообщения в лог (true = писать).
//   • enableValidation  — выполнять ли дополнительные проверки (assert).
//                          В релизных сборках обычно отключают для скорости.
//   • logLevel          — минимальный уровень сообщений (см. константы ниже).
//   • applicationName   — имя приложения (для заголовков логов, метаданных).
//
// Соответствие logLevel и LogLevel из Logger.hpp:
//   0 → Debug (самый подробный)
//   1 → Debug
//   2 → Info     ← значение по умолчанию
//   3 → Warning
//   4 → Error
//   5 → Critical
//
// Использование:
//   EngineConfig config;
//   config.enableLogging   = true;
//   config.enableValidation = true;
//   config.logLevel        = 1;               // Debug
//   config.applicationName = "MIR4D";
//   Engine engine(config);
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <string>

namespace mir {

struct EngineConfig {
    // ── Логирование ──────────────────────────────────────────
    // true  — сообщения уровня logLevel и выше будут записываться.
    // false — логирование полностью отключено.
    bool enableLogging = true;

    // ── Валидация ────────────────────────────────────────────
    // true  — в отладочных сборках будут срабатывать дополнительные проверки.
    // false — проверки отключены (рекомендуется для релизной сборки).
    bool enableValidation = true;

    // ── Уровень логирования ──────────────────────────────────
    // Минимальный уровень сообщений, которые будут записаны.
    // Значения:
    //   0 или 1 — Debug
    //   2       — Info (по умолчанию)
    //   3       — Warning
    //   4       — Error
    //   5       — Critical
    // Значения вне диапазона [0..5] будут приведены к Info.
    int logLevel = 2;

    // ── Имя приложения ───────────────────────────────────────
    // Используется в заголовках логов, именах файлов и метаданных.
    std::string applicationName = "MirEngine";

    // ── Вспомогательные константы (для удобства) ─────────────
    static constexpr int LogLevelDebug    = 1;
    static constexpr int LogLevelInfo     = 2;
    static constexpr int LogLevelWarning  = 3;
    static constexpr int LogLevelError    = 4;
    static constexpr int LogLevelCritical = 5;
};

} // namespace mir