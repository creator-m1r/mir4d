// MirUI/Designer/Commands/AddWidgetCommand.hpp
// 🧩 Команда «Добавить виджет» — финальная версия.
// Использует новый WidgetTree с автоматической регистрацией,
// а также создаёт конкретные подклассы виджетов (Button, Label, …),
// которые умеют работать со свойствами через setProperty/getProperty.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Commands/CommandHistory.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetType.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../Document/UIDocument.hpp"
#include "../../Widgets/Button/Button.hpp"    // конкретные виджеты
#include "../../Widgets/Label/Label.hpp"
#include "../../Widgets/Toolbar/Toolbar.hpp"
#include "../../Widgets/Tree/Tree.hpp"
#include "../../Widgets/PropertyGrid/PropertyGrid.hpp"
#include "../../Widgets/Viewport/Viewport.hpp"
#include <memory>
#include <string>
#include <stdexcept>

namespace MirUI {

class AddWidgetCommand : public ICommand {
public:
    AddWidgetCommand(UIDocument& doc,
                     WidgetType type,
                     WidgetID parentId)
        : m_doc(doc)
        , m_type(type)
        , m_parentId(parentId)
        , m_createdId()
    {}

    bool execute() override {
        // 1. Найти родителя.
        Widget* parent = m_doc.widgetTree().find(m_parentId);
        if (!parent) {
            return false;
        }

        // 2. Создать конкретный виджет нужного типа через фабрику.
        Widget* newWidget = createWidgetByType(m_type);
        if (!newWidget) {
            return false;
        }
        m_createdId = newWidget->id();

        // 3. Добавить к родителю (это также регистрирует в индексе через WidgetTree).
        parent->addChild(newWidget);
        // Явно регистрируем на случай, если addChild не делает этого автоматически.
        m_doc.widgetTree().registerWidget(newWidget);

        // 4. Если виджет был корнем ранее (что маловероятно), ничего особого.
        // 5. Помечаем документ изменённым.
        m_doc.setModified(true);
        return true;
    }

    bool undo() override {
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
        delete child;
        m_doc.setModified(true);
        return true;
    }

    [[nodiscard]] std::string description() const override {
        switch (m_type) {
            case WidgetType::Button:       return "Добавить кнопку";
            case WidgetType::Label:        return "Добавить надпись";
            case WidgetType::Tree:         return "Добавить дерево";
            case WidgetType::PropertyGrid: return "Добавить инспектор";
            case WidgetType::Toolbar:      return "Добавить панель инструментов";
            case WidgetType::Viewport:     return "Добавить вьюпорт";
            case WidgetType::DockPanel:    return "Добавить панель";
            case WidgetType::Panel:        return "Добавить контейнер";
            default:                        return "Добавить виджет";
        }
    }

private:
    UIDocument& m_doc;
    WidgetType  m_type;
    WidgetID    m_parentId;
    WidgetID    m_createdId;

    static Widget* createWidgetByType(WidgetType type) {
        // Создаём реальные виджеты с их конструкторами.
        switch (type) {
            case WidgetType::Button:       return new Button();
            case WidgetType::Label:        return new Label();
            case WidgetType::Toolbar:      return new Toolbar();
            case WidgetType::Tree:         return new Tree();
            case WidgetType::PropertyGrid: return new PropertyGrid();
            case WidgetType::Viewport:     return new Viewport();
            case WidgetType::DockPanel:    return new Widget(WidgetType::DockPanel);
            case WidgetType::Panel:        return new Widget(WidgetType::Panel);
            case WidgetType::Window:       return new Widget(WidgetType::Window);
            case WidgetType::Ribbon:       return new Widget(WidgetType::Ribbon);
            case WidgetType::Timeline:     return new Widget(WidgetType::Timeline);
            default:                       return nullptr;
        }
    }
};

} // namespace MirUI