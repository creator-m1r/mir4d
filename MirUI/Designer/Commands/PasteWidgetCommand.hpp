// MirUI/Designer/Commands/PasteWidgetCommand.hpp
// 📋 Команда «Вставить виджет» — создаёт копию из буфера обмена.
//
// Когда пользователь нажимает Cmd+V, эта команда читает глобальный
// WidgetClipboard, в котором CopyWidgetCommand сохранила свойства
// скопированного виджета, и создаёт новый виджет с теми же параметрами.
//
// Алгоритм:
//   1. Проверяем, есть ли что-то в WidgetClipboard.
//   2. Создаём новый виджет того же типа через WidgetFactory.
//   3. Применяем все сохранённые свойства (текст, размеры, цвет…).
//   4. Слегка смещаем позицию (x+20, y+20), чтобы было видно, что это копия.
//   5. Добавляем в того же родителя (или в корень окна, если родитель удалён).
//   6. Регистрируем в WidgetTree.
//
// Команда полностью поддерживает Undo: при отмене созданный виджет удаляется.
// Таким образом, вставка — это такое же обратимое действие, как и всё остальное.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Widget/WidgetFactory.hpp"
#include "../Document/UIDocument.hpp"
#include "WidgetClipboard.hpp"
#include <memory>
#include <string>

namespace MirUI {

class PasteWidgetCommand : public ICommand {
public:
    // Конструктор:
    //   doc      — документ, в который вставляем
    //   parentId — ID виджета-родителя, в который добавляем копию
    //              (если 0 — добавляем в корень окна)
    PasteWidgetCommand(UIDocument& doc, WidgetID parentId = WidgetID{})
        : m_doc(doc)
        , m_parentId(parentId)
        , m_createdId() // будет заполнен при execute
    {}

    // ── execute() — вставить виджет ──────────────────────────
    bool execute() override {
        WidgetClipboard& clipboard = WidgetClipboard::instance();
        if (!clipboard.hasContent()) {
            return false; // нечего вставлять
        }

        // Находим родителя. Если не указан или не найден, используем корень.
        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) {
            parent = m_doc.widgetTree().root();
        }
        if (!parent) {
            return false; // нет даже корня — некуда вставлять
        }
        m_parentId = parent->id(); // запоминаем для undo

        // Создаём новый виджет того же типа через фабрику.
        WidgetType type = clipboard.type();
        auto widgetPtr = WidgetFactory::create(type);
        if (!widgetPtr) {
            return false; // фабрика не смогла создать
        }
        Widget* newWidget = widgetPtr.get();

        // Применяем все сохранённые свойства.
        const auto& props = clipboard.properties();
        for (const auto& [key, val] : props) {
            newWidget->setProperty(key, val);
        }

        // Немного смещаем, чтобы копия не наложилась на оригинал.
        Rect bounds = newWidget->bounds();
        bounds.x += 20.0;
        bounds.y += 20.0;
        newWidget->setBounds(bounds);

        // Добавляем виджет к родителю.
        parent->addChild(widgetPtr.release()); // передаём владение родителю
        m_createdId = newWidget->id();

        // Регистрируем в индексе.
        m_doc.widgetTree().registerWidget(newWidget);

        m_doc.setModified(true);
        return true;
    }

    // ── undo() — удалить вставленный виджет ─────────────────
    bool undo() override {
        if (m_createdId.value() == 0) return false;

        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) return false;

        Widget* child = nullptr;
        for (Widget* c : parent->children()) {
            if (c->id() == m_createdId) {
                child = c;
                break;
            }
        }
        if (!child) return false;

        parent->removeChild(m_createdId);
        m_doc.widgetTree().unregisterWidget(m_createdId);
        delete child; // мы владеем, удаляем
        m_createdId = WidgetID{};

        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        return "Вставить виджет из буфера";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_parentId;
    WidgetID    m_createdId; // ID созданного виджета (для undo)
};

} // namespace MirUI