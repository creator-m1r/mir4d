// MirUI/Designer/Commands/CopyWidgetCommand.hpp
// 📋 Команда «Копировать виджет» — помещает описание виджета в буфер обмена.
//
// Когда пользователь выделяет кнопку и нажимает Cmd+C, вызывается эта команда.
// Она не изменяет дерево виджетов, а только сохраняет внутреннее представление
// (сериализованные свойства) выделенного виджета в специальный объект WidgetClipboard.
// Затем команда PasteWidgetCommand сможет прочитать этот буфер и создать
// точную копию виджета в нужном месте.
//
// Почему это команда, если она ничего не меняет? Потому что в будущем копирование
// может быть сложнее: например, копировать несколько виджетов, вырезать (Cut)
// с удалением оригинала. Тогда копирование станет частью большой операции,
// и её можно будет отменять/повторять через Undo/Redo. Пока мы делаем простой вариант.
//
// Как работает:
//   1. Находим виджет по ID.
//   2. Собираем все его свойства (текст, размеры, цвет, шрифт...) в карту.
//   3. Сохраняем эту карту и тип виджета в глобальный WidgetClipboard.
//   4. Если виджет не найден, ничего не делаем.
//
// Используется:
//   • DesignerCanvas при получении системного события копирования.
//   • В будущем — через CommandBus, чтобы разные части системы могли копировать.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp" // ICommand
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"
#include "WidgetClipboard.hpp" // наш внутренний буфер обмена
#include <memory>
#include <string>

namespace MirUI {

class CopyWidgetCommand : public ICommand {
public:
    // Конструктор принимает документ и ID копируемого виджета.
    CopyWidgetCommand(UIDocument& doc, WidgetID widgetId)
        : m_doc(doc)
        , m_widgetId(widgetId)
    {}

    // ── execute() — выполнить копирование ────────────────────
    bool execute() override {
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (!widget) return false; // виджет не найден

        // Собираем все свойства виджета в карту.
        std::unordered_map<std::string, StateValue> props;

        // Стандартные свойства, которые есть у любого виджета.
        props["type"] = StateValue(static_cast<int64_t>(static_cast<int>(widget->type())));
        props["name"] = StateValue(widget->name());
        props["visible"] = StateValue(widget->isVisible());
        props["enabled"] = StateValue(widget->isEnabled());
        // Геометрия
        props["x"] = StateValue(widget->bounds().x);
        props["y"] = StateValue(widget->bounds().y);
        props["width"] = StateValue(widget->bounds().width);
        props["height"] = StateValue(widget->bounds().height);

        // Все дополнительные свойства (текст, цвет, шрифт, скругление…)
        const auto& allProps = widget->allProperties();
        for (const auto& [key, val] : allProps) {
            // Пропускаем те, которые уже добавили, чтобы не дублировать.
            if (key == "type" || key == "name" || key == "visible" || key == "enabled" ||
                key == "x" || key == "y" || key == "width" || key == "height") {
                continue;
            }
            props[key] = val;
        }

        // Помещаем собранные свойства в глобальный буфер обмена.
        WidgetClipboard::instance().setContent(widget->type(), std::move(props));

        // Эта команда не меняет документ, поэтому флаг modified не трогаем.
        // Но в будущем, если будет Cut, изменим.
        return true;
    }

    // ── undo() — отменить копирование (обычно ничего не делаем) ──
    bool undo() override {
        // Копирование необратимо в том смысле, что буфер уже изменён.
        // Для простоты ничего не делаем. В будущем можно сохранять старый буфер.
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Копировать виджет";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
};

} // namespace MirUI