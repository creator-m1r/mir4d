// MirUI/Core/Theme/ThemeID.hpp
// 🏷️ Уникальный идентификатор темы — строковый ключ, который однозначно
//    определяет тему в системе MirUI.
//
// ThemeID используется для регистрации, поиска и переключения тем.
// Примеры значений:
//   "mir.light"       — стандартная светлая тема
//   "mir.dark"        — стандартная тёмная тема
//   "mir4d.engineering" — инженерная тема для САПР МИР 4D
//   "user.custom.001" — пользовательская тема, созданная в Designer
//
// Идентификатор хранится как обычная строка, что позволяет:
//   • легко добавлять новые темы без изменения кода ядра;
//   • использовать человекочитаемые имена в файлах конфигурации;
//   • сравнивать и хешировать темы для быстрого поиска.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include <functional>

namespace MirUI {

class ThemeID {
public:
    // ── Конструкторы ──────────────────────────────────────────
    ThemeID() = default;
    explicit ThemeID(std::string value) : m_value(std::move(value)) {}

    // ── Доступ к строковому значению ──────────────────────────
    [[nodiscard]] const std::string& value() const { return m_value; }

    // ── Операторы сравнения ───────────────────────────────────
    friend bool operator==(const ThemeID& a, const ThemeID& b) {
        return a.m_value == b.m_value;
    }
    friend bool operator!=(const ThemeID& a, const ThemeID& b) {
        return a.m_value != b.m_value;
    }
    friend bool operator<(const ThemeID& a, const ThemeID& b) {
        return a.m_value < b.m_value;
    }

private:
    std::string m_value;   // строковый идентификатор (например, "mir.dark")
};

} // namespace MirUI

// ── Специализация std::hash для использования в unordered_map ──
namespace std {
template <>
struct hash<MirUI::ThemeID> {
    std::size_t operator()(const MirUI::ThemeID& id) const {
        return hash<std::string>{}(id.value());
    }
};
} // namespace std