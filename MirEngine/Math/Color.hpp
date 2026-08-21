// MirEngine/Math/Color.hpp
// 🎨 Цвет — математическое представление цвета в формате RGBA.
//
// Color описывает цвет с четырьмя компонентами: красный (r), зелёный (g),
// синий (b) и альфа-канал (a) — прозрачность. Каждый компонент находится
// в диапазоне [0.0, 1.0], где 0.0 означает отсутствие, а 1.0 — максимум.
//
// Этот класс используется во всей системе визуализации MirEngine:
//   • Материалы и поверхности (цвет заливки, цвет границы).
//   • Освещение и тени.
//   • Аннотации и размерные линии.
//   • Визуальные стили (цвет выделения, цвет сетки, оси координат).
//
// Color НЕ зависит от платформы: это просто набор чисел.
// При необходимости платформенные адаптеры (SwiftUI, WinUI) преобразуют
// его в нативные типы (NSColor, Windows::UI::Color и т.д.).
//
// Возможности:
//   • Создание из RGB или RGBA значений.
//   • Предопределённые константы (белый, чёрный, красный, зелёный, синий…).
//   • Преобразование в HEX-строку и обратно.
//   • Линейная интерполяция (lerp) для плавных переходов.
//   • Получение более светлого или тёмного оттенка.
//   • Проверка на прозрачность.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "../Core/Types/Scalar.hpp"   // mir::Scalar = double
#include <string>                      // для HEX-строк
#include <sstream>                     // для форматирования HEX
#include <iomanip>                     // std::setw, std::setfill
#include <cmath>                       // std::round
#include <algorithm>                   // std::clamp
#include <stdexcept>                   // std::invalid_argument

namespace mir {

class Color {
public:
    // ── Компоненты цвета ─────────────────────────────────────
    Scalar r = 0.0;   // красный   (0.0 — отсутствует, 1.0 — максимум)
    Scalar g = 0.0;   // зелёный
    Scalar b = 0.0;   // синий
    Scalar a = 1.0;   // альфа (прозрачность): 0.0 — полностью прозрачный, 1.0 — непрозрачный

    // ── Конструкторы ─────────────────────────────────────────
    constexpr Color() noexcept = default;

    // Создаёт цвет с заданными компонентами.
    // Значения автоматически зажимаются в диапазоне [0.0, 1.0].
    constexpr Color(Scalar r, Scalar g, Scalar b, Scalar a = 1.0) noexcept
        : r(std::clamp(r, Scalar(0), Scalar(1)))
        , g(std::clamp(g, Scalar(0), Scalar(1)))
        , b(std::clamp(b, Scalar(0), Scalar(1)))
        , a(std::clamp(a, Scalar(0), Scalar(1)))
    {}

    // ── Статические фабрики ──────────────────────────────────

    // Создать цвет из RGB (альфа = 1.0).
    [[nodiscard]] static constexpr Color rgb(Scalar r, Scalar g, Scalar b) noexcept {
        return Color{r, g, b, 1.0};
    }

    // Создать цвет из RGBA.
    [[nodiscard]] static constexpr Color rgba(Scalar r, Scalar g, Scalar b, Scalar a) noexcept {
        return Color{r, g, b, a};
    }

    // ── Предопределённые константы ───────────────────────────
    [[nodiscard]] static constexpr Color transparent() noexcept { return {0, 0, 0, 0}; }
    [[nodiscard]] static constexpr Color white()   noexcept { return {1, 1, 1, 1}; }
    [[nodiscard]] static constexpr Color black()   noexcept { return {0, 0, 0, 1}; }
    [[nodiscard]] static constexpr Color red()     noexcept { return {1, 0, 0, 1}; }
    [[nodiscard]] static constexpr Color green()   noexcept { return {0, 1, 0, 1}; }
    [[nodiscard]] static constexpr Color blue()    noexcept { return {0, 0, 1, 1}; }
    [[nodiscard]] static constexpr Color yellow()  noexcept { return {1, 1, 0, 1}; }
    [[nodiscard]] static constexpr Color cyan()    noexcept { return {0, 1, 1, 1}; }
    [[nodiscard]] static constexpr Color magenta() noexcept { return {1, 0, 1, 1}; }
    [[nodiscard]] static constexpr Color gray()    noexcept { return {0.5, 0.5, 0.5, 1}; }

    // ── Преобразование в HEX-строку и обратно ────────────────

    // Возвращает HEX-строку в формате "#RRGGBBAA" (8 шестнадцатеричных цифр).
    [[nodiscard]] std::string toHex() const {
        std::ostringstream oss;
        oss << "#"
            << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << static_cast<int>(std::round(r * 255))
            << std::setw(2) << static_cast<int>(std::round(g * 255))
            << std::setw(2) << static_cast<int>(std::round(b * 255))
            << std::setw(2) << static_cast<int>(std::round(a * 255));
        return oss.str();
    }

    // Создать цвет из HEX-строки.
    // Поддерживаются форматы: "#RGB", "#RGBA", "#RRGGBB", "#RRGGBBAA".
    [[nodiscard]] static Color fromHex(const std::string& hex) {
        if (hex.empty() || hex[0] != '#') {
            throw std::invalid_argument("Color::fromHex: строка должна начинаться с '#'");
        }

        std::string digits = hex.substr(1);
        // Удаляем пробелы, если есть
        digits.erase(std::remove_if(digits.begin(), digits.end(), ::isspace), digits.end());

        Scalar r, g, b, a = 1.0;
        if (digits.size() == 3) {
            // #RGB → каждый символ удваивается
            r = std::stoi(digits.substr(0,1), nullptr, 16) / 15.0;
            g = std::stoi(digits.substr(1,1), nullptr, 16) / 15.0;
            b = std::stoi(digits.substr(2,1), nullptr, 16) / 15.0;
        } else if (digits.size() == 4) {
            // #RGBA
            r = std::stoi(digits.substr(0,1), nullptr, 16) / 15.0;
            g = std::stoi(digits.substr(1,1), nullptr, 16) / 15.0;
            b = std::stoi(digits.substr(2,1), nullptr, 16) / 15.0;
            a = std::stoi(digits.substr(3,1), nullptr, 16) / 15.0;
        } else if (digits.size() == 6) {
            // #RRGGBB
            r = std::stoi(digits.substr(0,2), nullptr, 16) / 255.0;
            g = std::stoi(digits.substr(2,2), nullptr, 16) / 255.0;
            b = std::stoi(digits.substr(4,2), nullptr, 16) / 255.0;
        } else if (digits.size() == 8) {
            // #RRGGBBAA
            r = std::stoi(digits.substr(0,2), nullptr, 16) / 255.0;
            g = std::stoi(digits.substr(2,2), nullptr, 16) / 255.0;
            b = std::stoi(digits.substr(4,2), nullptr, 16) / 255.0;
            a = std::stoi(digits.substr(6,2), nullptr, 16) / 255.0;
        } else {
            throw std::invalid_argument("Color::fromHex: неверная длина HEX-строки (ожидается 3, 4, 6 или 8 цифр после #)");
        }
        return Color{r, g, b, a};
    }

    // ── Операции с цветом ────────────────────────────────────

    // Копия цвета с изменённой альфой.
    [[nodiscard]] constexpr Color withAlpha(Scalar newAlpha) const noexcept {
        return Color{r, g, b, newAlpha};
    }

    // Линейная интерполяция между двумя цветами.
    [[nodiscard]] static Color lerp(const Color& c1, const Color& c2, Scalar t) noexcept {
        t = std::clamp(t, Scalar(0), Scalar(1));
        return Color{
            c1.r + (c2.r - c1.r) * t,
            c1.g + (c2.g - c1.g) * t,
            c1.b + (c2.b - c1.b) * t,
            c1.a + (c2.a - c1.a) * t
        };
    }

    // Более светлый оттенок (смешивание с белым).
    [[nodiscard]] Color lighter(Scalar factor = 0.3) const noexcept {
        return lerp(*this, white(), factor);
    }

    // Более тёмный оттенок (смешивание с чёрным).
    [[nodiscard]] Color darker(Scalar factor = 0.3) const noexcept {
        return lerp(*this, black(), factor);
    }

    // ── Проверки ─────────────────────────────────────────────
    [[nodiscard]] constexpr bool isTransparent() const noexcept {
        return a == 0.0;
    }

    [[nodiscard]] constexpr bool isOpaque() const noexcept {
        return a == 1.0;
    }

    // ── Сравнение ────────────────────────────────────────────
    friend constexpr bool operator==(const Color& a, const Color& b) noexcept {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    }
    friend constexpr bool operator!=(const Color& a, const Color& b) noexcept {
        return !(a == b);
    }
};

} // namespace mir