// MirUI/Widgets/Slider/Slider.hpp
// 🎚️ Виджет «Ползунок» (Slider) — выбор числа в заданном диапазоне.
//
// Slider отображает горизонтальную дорожку, по которой можно двигать «бегунок».
// Пользователь тянет бегунок мышкой, и значение меняется плавно или с шагом.
// Это идеальный виджет для настройки громкости, яркости, размера, скорости...
//
// Основные свойства:
//   • value       — текущее значение (double).
//   • minValue    — минимальное возможное значение.
//   • maxValue    — максимальное возможное значение.
//   • step        — шаг изменения (0 = любое значение).
//   • enabled     — можно ли двигать ползунок.
//   • orientation — направление: "horizontal" или "vertical".
//
// Как и все виджеты MirUI, Slider — чистое C++ описание.
// SwiftUI/WinUI адаптеры читают свойства и создают нативные элементы:
// Slider в SwiftUI, Slider в WinUI.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>
#include <algorithm>

namespace MirUI {

class Slider : public Widget {
public:
    // ── Конструктор ──────────────────────────────────────────
    explicit Slider(double minValue = 0.0, double maxValue = 100.0, double initialValue = 50.0)
        : Widget(WidgetType::Slider)  // тип должен быть добавлен в WidgetType.hpp
    {
        setProperty("value", StateValue(initialValue));
        setProperty("minValue", StateValue(minValue));
        setProperty("maxValue", StateValue(maxValue));
        setProperty("step", StateValue(0.0));
        setProperty("enabled", StateValue(true));
        setProperty("orientation", StateValue(std::string("horizontal")));
        // Ползунок обычно растягивается по ширине и имеет фиксированную высоту.
        setLayoutData(LayoutData::fixed(200, 20));
    }

    // ── Значение ─────────────────────────────────────────────
    void setValue(double value) {
        double minVal = getMinValue();
        double maxVal = getMaxValue();
        value = std::clamp(value, minVal, maxVal);
        double st = getStep();
        if (st > 0.0) {
            // Приводим к ближайшему шагу.
            value = minVal + std::round((value - minVal) / st) * st;
        }
        setProperty("value", StateValue(value));
    }

    [[nodiscard]] double getValue() const {
        auto val = getProperty("value");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 0.0;
    }

    // ── Минимальное значение ─────────────────────────────────
    void setMinValue(double minValue) {
        setProperty("minValue", StateValue(minValue));
        // Убедимся, что текущее значение не вышло за границы.
        if (getValue() < minValue) setValue(minValue);
    }

    [[nodiscard]] double getMinValue() const {
        auto val = getProperty("minValue");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 0.0;
    }

    // ── Максимальное значение ────────────────────────────────
    void setMaxValue(double maxValue) {
        setProperty("maxValue", StateValue(maxValue));
        if (getValue() > maxValue) setValue(maxValue);
    }

    [[nodiscard]] double getMaxValue() const {
        auto val = getProperty("maxValue");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 100.0;
    }

    // ── Шаг ──────────────────────────────────────────────────
    void setStep(double step) {
        setProperty("step", StateValue(step));
    }

    [[nodiscard]] double getStep() const {
        auto val = getProperty("step");
        if (val.has_value() && std::holds_alternative<double>(*val)) {
            return std::get<double>(*val);
        }
        return 0.0;
    }

    // ── Ориентация ───────────────────────────────────────────
    void setOrientation(const std::string& orientation) {
        setProperty("orientation", StateValue(orientation));
    }

    [[nodiscard]] std::string getOrientation() const {
        auto val = getProperty("orientation");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "horizontal";
    }

    // ── Универсальный доступ к свойствам ─────────────────────
    bool setProperty(const std::string& name, const StateValue& value) override {
        if (name == "value" && std::holds_alternative<double>(value)) {
            double v = std::get<double>(value);
            v = std::clamp(v, getMinValue(), getMaxValue());
            m_properties[name] = StateValue(v);
            return true;
        }
        if (name == "minValue" && std::holds_alternative<double>(value)) {
            m_properties[name] = value;
            // Проверяем текущее значение.
            if (getValue() < std::get<double>(value)) setValue(std::get<double>(value));
            return true;
        }
        if (name == "maxValue" && std::holds_alternative<double>(value)) {
            m_properties[name] = value;
            if (getValue() > std::get<double>(value)) setValue(std::get<double>(value));
            return true;
        }
        if (name == "step" && std::holds_alternative<double>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "orientation" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "value" || name == "minValue" || name == "maxValue" ||
            name == "step" || name == "orientation") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }
};

} // namespace MirUI