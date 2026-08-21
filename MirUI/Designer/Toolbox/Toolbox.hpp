
#pragma once

#include "ToolboxModel.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/AddWidgetCommand.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <memory>

namespace MirUI {

class Toolbox {
public:

    explicit Toolbox(UIDocument& document)
        : m_doc(document)
        , m_model(std::make_unique<ToolboxModel>())
    {

        populateDefaultItems();
    }

    [[nodiscard]] ToolboxModel& model() { return *m_model; }
    [[nodiscard]] const ToolboxModel& model() const { return *m_model; }

    WidgetID addWidget(WidgetType itemType, WidgetID parentId) {

        auto cmd = std::make_unique<AddWidgetCommand>(m_doc, itemType, parentId);

        m_doc.history().execute(std::move(cmd));

        Widget* parent = m_doc.widgetTree().find(parentId);
        if (!parent || parent->children().empty()) {
            return WidgetID{};
        }

        Widget* lastChild = parent->children().back();
        return lastChild->id();
    }

    WidgetID addWidgetToRoot(WidgetType itemType) {
        Widget* root = m_doc.widgetTree().root();
        if (!root) {
            return WidgetID{};
        }
        return addWidget(itemType, root->id());
    }

    [[nodiscard]] const ToolboxItem* findItemByType(WidgetType type) const {
        return m_model->findByType(type);
    }

private:
    UIDocument& m_doc;
    std::unique_ptr<ToolboxModel> m_model;

    void populateDefaultItems() {
        m_model->addItem(ToolboxItem{
            "Button",
            "Кнопка",
            WidgetType::Button,
            IconID("button"),
            "Нажимаемый элемент с текстом, иконкой и командой"
        });
        m_model->addItem(ToolboxItem{
            "Label",
            "Надпись",
            WidgetType::Label,
            IconID("label"),
            "Простой текст"
        });
        m_model->addItem(ToolboxItem{
            "Toolbar",
            "Панель инструментов",
            WidgetType::Toolbar,
            IconID("toolbar"),
            "Контейнер для кнопок и других элементов управления"
        });
        m_model->addItem(ToolboxItem{
            "Tree",
            "Дерево",
            WidgetType::Tree,
            IconID("tree"),
            "Иерархический список (навигатор, структура проекта)"
        });
        m_model->addItem(ToolboxItem{
            "PropertyGrid",
            "Инспектор свойств",
            WidgetType::PropertyGrid,
            IconID("propertygrid"),
            "Панель для редактирования свойств выделенного объекта"
        });
        m_model->addItem(ToolboxItem{
            "Viewport",
            "Вьюпорт",
            WidgetType::Viewport,
            IconID("viewport"),
            "Область 3D-вида"
        });
        m_model->addItem(ToolboxItem{
            "DockPanel",
            "Стыкуемая панель",
            WidgetType::DockPanel,
            IconID("dockpanel"),
            "Панель, которую можно прикреплять к краям окна"
        });
        m_model->addItem(ToolboxItem{
            "Panel",
            "Контейнер",
            WidgetType::Panel,
            IconID("panel"),
            "Пустой контейнер для группировки других виджетов"
        });
    }
};

}