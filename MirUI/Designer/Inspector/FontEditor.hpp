// MirUI/Designer/Inspector/FontEditor.hpp
// 🔤 Редактор шрифта для инспектора свойств (PropertyGrid).
//
// Когда ты выбираешь виджет и в инспекторе видишь свойство «шрифт»,
// FontEditor отображает текущий шрифт (семейство, размер, начертание, стиль)
// и позволяет изменить его через встроенный диалог или с помощью
// прямого ввода значений. Все изменения применяются через команду
// ChangePropertyCommand и попадают в историю Undo/Redo.
//
// FontEditor хранит Font как объект, а в свойстве виджета —
// строковое представление (семейство;размер;начертание;стиль).
// Рендерер (SwiftUI, WinUI) отображает настроенный шрифт, читая
// это свойство.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Foundation/Typography/Font.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace MirUI {

class FontEditor {
public:
    // Конструктор:
    //   doc           — документ, содержащий виджет
    //   widgetId      — ID редактируемого виджета
    //   propertyName  — имя свойства (например, "font")
    FontEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {
        // Загружаем текущее значение из виджета.
        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            auto value = widget->getProperty(propertyName);
            if (value.has_value() && std::holds_alternative<std::string>(*value)) {
                m_currentFont = deserializeFont(std::get<std::string>(*value));
            }
        }
        // Если шрифт не задан, используем системный шрифт по умолчанию.
        if (m_currentFont.family.empty()) {
            m_currentFont = Font("System", 14.0, FontWeight::Regular, FontStyle::Normal);
        }
    }

    // ── Текущий шрифт ────────────────────────────────────────
    [[nodiscard]] Font currentFont() const { return m_currentFont; }

    // Установить новый шрифт напрямую (без диалога) — полезно при программных изменениях.
    void setFont(const Font& newFont) {
        if (newFont == m_currentFont) return;

        // Превращаем Font в строку для хранения в свойстве.
        std::string serialized = serializeFont(newFont);

        // Создаём команду изменения свойства.
        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName,
            StateValue(serialized)
        );
        // Выполняем команду (она попадёт в историю).
        m_doc.history().execute(std::move(cmd));

        // Обновляем локальное значение.
        m_currentFont = newFont;
    }

    // ── Показать диалог выбора шрифта ─────────────────────────
    // В будущем здесь будет вызов платформенного диалога шрифтов.
    // На этапе MVP просто переключает тестовые шрифты.
    // Возвращает true, если пользователь выбрал шрифт (не нажал Отмена).
    bool showFontDialog() {
        // Заглушка: циклически меняем шрифт на тестовые варианты.
        static int step = 0;
        Font testFonts[] = {
            Font("System", 12.0, FontWeight::Regular, FontStyle::Normal),
            Font("System", 16.0, FontWeight::Bold, FontStyle::Italic),
            Font("Menlo",   14.0, FontWeight::Medium, FontStyle::Normal),
            Font("Georgia", 18.0, FontWeight::SemiBold, FontStyle::Normal),
        };
        setFont(testFonts[step % 4]);
        ++step;
        return true; // диалог «подтверждён»
    }

    // ── Сброс к значению по умолчанию ────────────────────────
    void resetToDefault() {
        setFont(Font("System", 14.0, FontWeight::Regular, FontStyle::Normal));
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    Font        m_currentFont;

    // ── Сериализация Font в строку ───────────────────────────
    // Формат: "family;size;weight;style"
    //   family — название шрифта (без ';')
    //   size   — размер в пунктах (double)
    //   weight — числовое значение FontWeight (int)
    //   style  — числовое значение FontStyle (int)
    static std::string serializeFont(const Font& font) {
        return font.family + ";"
             + std::to_string(font.size) + ";"
             + std::to_string(static_cast<int>(font.weight)) + ";"
             + std::to_string(static_cast<int>(font.style));
    }

    // ── Десериализация строки в Font ─────────────────────────
    static Font deserializeFont(const std::string& data) {
        Font result("System", 14.0, FontWeight::Regular, FontStyle::Normal);
        // Простейший парсинг строки через ';'
        std::stringstream ss(data);
        std::string token;
        std::vector<std::string> parts;
        while (std::getline(ss, token, ';')) {
            parts.push_back(token);
        }

        if (parts.size() >= 1) result.family = parts[0];
        if (parts.size() >= 2) {
            try { result.size = std::stod(parts[1]); } catch (...) {}
        }
        if (parts.size() >= 3) {
            try {
                int w = std::stoi(parts[2]);
                result.weight = static_cast<FontWeight>(w);
            } catch (...) {}
        }
        if (parts.size() >= 4) {
            try {
                int s = std::stoi(parts[3]);
                result.style = static_cast<FontStyle>(s);
            } catch (...) {}
        }
        return result;
    }
};

} // namespace MirUI