// MirUI/Designer/Inspector/ColorEditor.hpp
// 🎨 Редактор цвета для инспектора свойств (PropertyGrid).
//
// Когда ты выбираешь виджет и в инспекторе видишь свойство «цвет фона»,
// появляется маленький прямоугольник с текущим цветом и кнопка «...»,
// по которой открывается палитра. ColorEditor управляет этим процессом:
// он хранит выбранный цвет, умеет показывать диалог выбора (пока заглушка)
// и через команду ChangePropertyCommand обновляет свойство виджета,
// автоматически поддерживая Undo/Redo.
//
// Важно: ColorEditor НЕ рисует пиксели сам — он только хранит цвет
// и логику его изменения. Отображение цвета (цветной квадратик)
// делает платформенный рендерер через RenderCommandBuffer.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Foundation/Color/Color.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <functional>

namespace MirUI {

class ColorEditor {
public:
    // Конструктор: запоминаем документ, виджет и имя свойства,
    // с которым работает редактор (например, "background").
    ColorEditor(UIDocument& doc, WidgetID widgetId, const std::string& propertyName)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_propertyName(propertyName)
    {
        // Пытаемся загрузить текущее значение свойства из виджета.
        auto value = doc.widgetTree().find(widgetId)->getProperty(propertyName);
        if (value.has_value() && std::holds_alternative<std::string>(*value)) {
            // Цвет хранится как строка в формате "#RRGGBBAA" (пока).
            m_currentColor = Color::fromHex(std::get<std::string>(*value));
        } else {
            // Если свойство не задано, ставим прозрачный чёрный.
            m_currentColor = Color::transparent();
        }
    }

    // ── Текущий цвет ─────────────────────────────────────────
    [[nodiscard]] Color color() const { return m_currentColor; }

    // Установить цвет напрямую (без диалога) — полезно при программных изменениях.
    void setColor(const Color& newColor) {
        if (newColor == m_currentColor) return;

        // Создаём команду изменения свойства.
        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_propertyName,
            StateValue(newColor.toHex()) // сохраняем как HEX-строку
        );
        // Выполняем команду (она попадёт в историю).
        m_doc.history().execute(std::move(cmd));

        // Обновляем локальное значение.
        m_currentColor = newColor;
    }

    // ── Показать диалог выбора цвета ─────────────────────────
    // В будущем здесь будет вызов платформенной палитры цветов.
    // На этапе MVP просто устанавливает случайный цвет для демонстрации.
    // Возвращает true, если пользователь выбрал цвет (не нажал Отмена).
    bool showColorDialog() {
        // Заглушка: меняем цвет на тестовый (красный, зелёный, синий, белый по кругу).
        // В реальном приложении здесь будет вызов платформенного ColorPicker.
        static int step = 0;
        Color testColors[] = {
            Color::rgb(1.0f, 0.0f, 0.0f), // красный
            Color::rgb(0.0f, 1.0f, 0.0f), // зелёный
            Color::rgb(0.0f, 0.0f, 1.0f), // синий
            Color::white(),               // белый
        };
        setColor(testColors[step % 4]);
        ++step;
        return true; // диалог «подтверждён»
    }

    // ── Сброс к значению по умолчанию ────────────────────────
    void resetToDefault() {
        setColor(Color::transparent()); // или другой дефолтный
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    std::string m_propertyName;
    Color       m_currentColor;
};

} // namespace MirUI