// MirUI/Widgets/CheckBox/CheckBox.hpp
// ✅ Виджет «Флажок» (CheckBox) — переключатель с надписью.
//
// CheckBox — это виджет, который показывает квадратик с галочкой и текст рядом.
// Пользователь может щёлкать по нему, чтобы изменить состояние «включено/выключено».
// В отличие от кнопки-переключателя (Button с isToggle=true), флажок специально
// предназначен для булевых значений и имеет характерный внешний вид:
// пустой квадрат (выключен) или квадрат с галочкой (включен) + текст.
//
// Основные свойства:
//   • text    — текст, отображаемый справа от квадратика.
//   • checked — текущее состояние: true = галочка стоит, false = пусто.
//   • enabled — можно ли нажимать на флажок (если false, он серый и не активен).
//
// Как и все виджеты MirUI, CheckBox наследуется от базового Widget.
// Он хранит свои специфические свойства в универсальной карте m_properties
// и переопределяет setProperty/getProperty для удобного доступа.
//
// CheckBox не зависит от платформы — это чистое C++ описание.
// SwiftUI/WinUI адаптеры будут читать свойства "checked" и "text"
// и рисовать соответствующий нативный элемент (Toggle в SwiftUI, CheckBox в WinUI).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>

namespace MirUI {

class CheckBox : public Widget {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Создаёт флажок с заданным текстом. По умолчанию выключен.
    explicit CheckBox(const std::string& text = "Флажок")
        : Widget(WidgetType::CheckBox)   // WidgetType::CheckBox должен быть добавлен в WidgetType.hpp
    {
        // Устанавливаем стандартные свойства.
        setProperty("text", StateValue(text));
        setProperty("checked", StateValue(false));
        setProperty("enabled", StateValue(true));

        // Флажок обычно не имеет фиксированного размера — он подстраивается под текст.
        setLayoutData(LayoutData::fit());
    }

    // ── Текст ─────────────────────────────────────────────────
    void setText(const std::string& text) {
        setProperty("text", StateValue(text));
    }
    [[nodiscard]] std::string getText() const {
        auto val = getProperty("text");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "";
    }

    // ── Состояние (галочка) ──────────────────────────────────
    void setChecked(bool checked) {
        setProperty("checked", StateValue(checked));
    }
    [[nodiscard]] bool isChecked() const {
        auto val = getProperty("checked");
        if (val.has_value() && std::holds_alternative<bool>(*val)) {
            return std::get<bool>(*val);
        }
        return false;
    }

    // ── Переключение (toggle) ────────────────────────────────
    void toggle() {
        setChecked(!isChecked());
    }

    // ── Универсальный доступ к свойствам ─────────────────────
    bool setProperty(const std::string& name, const StateValue& value) override {
        // Обрабатываем специфические свойства, остальное — в базовый класс.
        if (name == "checked" && std::holds_alternative<bool>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "text" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "checked" || name == "text") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }
};

} // namespace MirUI