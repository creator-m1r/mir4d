// MirUI/Widgets/ComboBox/ComboBox.hpp
// 📋 Виджет «Выпадающий список» (ComboBox) — выбор одного значения из списка.
//
// ComboBox показывает текущее выбранное значение и, при нажатии,
// раскрывает список всех доступных вариантов. Пользователь может
// выбрать один из них. Это один из самых распространённых элементов
// интерфейса для выбора из набора опций (страна, цвет, категория…).
//
// Основные свойства:
//   • selectedIndex — индекс выбранного элемента (начиная с 0, -1 = ничего не выбрано).
//   • items         — список строк-вариантов, хранится как свойство "items"
//                     в виде строки с разделителем '|' (например, "Красный|Зелёный|Синий").
//   • enabled       — можно ли открывать список (если false, он серый).
//
// Как и все виджеты MirUI, ComboBox — это чистое C++ описание.
// SwiftUI/WinUI адаптеры читают свойства и создают нативные элементы:
// Picker в SwiftUI, ComboBox в WinUI.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

namespace MirUI {

class ComboBox : public Widget {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Создаёт выпадающий список с заданным набором элементов.
    // По умолчанию выбран первый элемент (индекс 0).
    explicit ComboBox(const std::vector<std::string>& initialItems = {})
        : Widget(WidgetType::ComboBox)  // тип должен быть добавлен в WidgetType.hpp
    {
        // Устанавливаем свойства по умолчанию.
        setItems(initialItems);
        setProperty("selectedIndex", StateValue(static_cast<int64_t>(initialItems.empty() ? -1 : 0)));
        setProperty("enabled", StateValue(true));

        // Выпадающий список обычно имеет фиксированную высоту и ширину.
        setLayoutData(LayoutData::fixed(200, 28));
    }

    // ── Элементы списка ──────────────────────────────────────
    // Хранятся внутри как строка с разделителем '|'.
    void setItems(const std::vector<std::string>& items) {
        // Превращаем вектор в строку через '|'.
        std::string joined;
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) joined += "|";
            // Экранируем разделитель: если в элементе есть '|', заменяем на "\|".
            std::string escaped = items[i];
            size_t pos = 0;
            while ((pos = escaped.find('|', pos)) != std::string::npos) {
                escaped.insert(pos, "\\");
                pos += 2;
            }
            joined += escaped;
        }
        setProperty("items", StateValue(joined));
    }

    [[nodiscard]] std::vector<std::string> getItems() const {
        auto val = getProperty("items");
        if (!val.has_value() || !std::holds_alternative<std::string>(*val)) {
            return {};
        }
        return splitEscaped(std::get<std::string>(*val), '|');
    }

    // ── Выбранный индекс ─────────────────────────────────────
    void setSelectedIndex(int64_t index) {
        setProperty("selectedIndex", StateValue(index));
    }

    [[nodiscard]] int64_t selectedIndex() const {
        auto val = getProperty("selectedIndex");
        if (val.has_value() && std::holds_alternative<int64_t>(*val)) {
            return std::get<int64_t>(*val);
        }
        return -1;
    }

    // ── Выбранный текст (удобный метод) ──────────────────────
    [[nodiscard]] std::string selectedText() const {
        int64_t idx = selectedIndex();
        auto items = getItems();
        if (idx >= 0 && static_cast<size_t>(idx) < items.size()) {
            return items[static_cast<size_t>(idx)];
        }
        return "";
    }

    // ── Универсальный доступ к свойствам ─────────────────────
    bool setProperty(const std::string& name, const StateValue& value) override {
        if (name == "items" && std::holds_alternative<std::string>(value)) {
            m_properties[name] = value;
            return true;
        }
        if (name == "selectedIndex" && std::holds_alternative<int64_t>(value)) {
            m_properties[name] = value;
            return true;
        }
        return Widget::setProperty(name, value);
    }

    std::optional<StateValue> getProperty(const std::string& name) const override {
        if (name == "items" || name == "selectedIndex") {
            auto it = m_properties.find(name);
            if (it != m_properties.end()) return it->second;
        }
        return Widget::getProperty(name);
    }

private:
    // Вспомогательная функция: разбивает строку по разделителю с учётом экранирования.
    static std::vector<std::string> splitEscaped(const std::string& str, char delimiter) {
        std::vector<std::string> result;
        std::string current;
        bool escape = false;
        for (char ch : str) {
            if (escape) {
                current += ch;
                escape = false;
            } else if (ch == '\\') {
                escape = true;
            } else if (ch == delimiter) {
                result.push_back(current);
                current.clear();
            } else {
                current += ch;
            }
        }
        result.push_back(current);
        return result;
    }
};

} // namespace MirUI