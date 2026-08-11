// MirUI/Widgets/TextField/TextField.hpp
// 📝 Виджет «Текстовое поле» (TextField) — поле для ввода текста пользователем.
//
// TextField позволяет пользователю вводить и редактировать одну строку текста.
// Это один из самых важных элементов интерфейса: через текстовые поля
// заполняют формы, вводят названия, ищут информацию.
//
// Основные свойства:
//   • text         — текущий текст, который видит и может редактировать пользователь.
//   • placeholder  — подсказка внутри поля, которая исчезает при начале ввода
//                    (например, «Введите имя»).
//   • enabled      — можно ли редактировать текст (если false, поле серое).
//   • readOnly     — если true, текст можно выделить и скопировать, но нельзя изменить.
//   • maxLength    — максимальное количество символов (0 = без ограничения).
//   • textAlignment — выравнивание текста внутри поля (Left, Center, Right).
//
// TextField, как и все виджеты MirUI, является чистым C++ описанием.
// SwiftUI/WinUI адаптеры будут читать его свойства и создавать
// соответствующие нативные элементы: TextField в SwiftUI, TextBox в WinUI.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>
#include <cstdint>

namespace MirUI {

class TextField : public Widget {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Создаёт текстовое поле с заданным текстом и placeholder'ом.
    explicit TextField(const std::string& initialText = "",
                       const std::string& placeholder = "Введите текст...")
        : Widget(WidgetType::TextField)  // тип должен быть добавлен в WidgetType.hpp
    {
        // Устанавливаем свойства по умолчанию.
        setProperty("text", StateValue(initialText));
        setProperty("placeholder", StateValue(placeholder));
        setProperty("enabled", StateValue(true));
        setProperty("readOnly", StateValue(false));
        setProperty("maxLength", StateValue(static_cast<int64_t>(0)));
        setProperty("textAlignment", StateValue(std::string("Left")));

        // Текстовое поле обычно имеет фиксированную высоту и растягивается по ширине.
        setLayoutData(LayoutData::fixed(200, 28)); // ширина 200, высота 28 пикселей по умолчанию
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

    // ── Подсказка (placeholder) ──────────────────────────────
    void setPlaceholder(const std::string& placeholder) {
        setProperty("placeholder", StateValue(placeholder));
    }
    [[nodiscard]] std::string getPlaceholder() const {
        auto val = getProperty("placeholder");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "";
    }

    // ── Только для чтения ────────────────────────────────────
    void setReadOnly(bool readOnly) {
        setProperty("readOnly", StateValue(readOnly));
    }
    [[nodiscard]] bool isReadOnly() const {
        auto val = getProperty("readOnly");
        if (val.has_value() && std::holds_alternative<bool>(*val)) {
            return std::get<bool>(*val);
        }
        return false;
    }

    // ── Максимальная длина ───────────────────────────────────
    void setMaxLength(int64_t maxLen) {
        setProperty("maxLength", StateValue(maxLen));
    }
    [[nodiscard]] int64_t maxLength() const {
        auto val = getProperty("maxLength");
        if (val.has_value() && std::holds_alternative<int64_t>(*val)) {
            return std::get<int64_t>(*val);
        }
        return 0;
    }

    // ── Выравнивание текста ──────────────────────────────────
    void setTextAlignment(const std::string& alignment) {
        setProperty("textAlignment", StateValue(alignment));
    }
    [[nodiscard]] std::string getTextAlignment() const {
        auto val = getProperty("textAlignment");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            return std::get<std::string>(*val);
        }
        return "Left";
    }

    // ── Универсальный доступ к свойствам ─────────────────────
    bool setProperty(const std::string& name, const StateValue& value) override {
        if (name == "text" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "placeholder" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "readOnly" && std::holds_alternative<bool>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "maxLength" && std::holds_alternative<int64_t>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "textAlignment" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "text" || name == "placeholder" ||
            name == "readOnly" || name == "maxLength" ||
            name == "textAlignment") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }
};

} // namespace MirUI