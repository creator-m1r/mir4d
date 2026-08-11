// MirUI/Designer/Toolbox/Toolbox.hpp
// 🧰 Панель инструментов (Toolbox) — контроллер, связывающий список доступных
// виджетов (ToolboxModel) с документом (UIDocument).
//
// Когда пользователь выбирает виджет в тулбоксе и перетаскивает его на холст
// (или дважды щёлкает по нему), Toolbox создаёт команду AddWidgetCommand
// и выполняет её через историю документа, автоматически поддерживая Undo/Redo.
// Он не рисует сам панель — только предоставляет данные и действия.
// Отображение (иконки, названия, перетаскивание) делает платформенный рендерер.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "ToolboxModel.hpp"
#include "../Document/UIDocument.hpp"
#include "../Commands/AddWidgetCommand.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <memory>

namespace MirUI {

class Toolbox {
public:
    // Конструктор принимает документ, в который будут добавляться новые виджеты.
    explicit Toolbox(UIDocument& document)
        : m_doc(document)
        , m_model(std::make_unique<ToolboxModel>())
    {
        // Наполняем модель стандартным набором виджетов.
        populateDefaultItems();
    }

    // ── Модель (список доступных виджетов) ───────────────────
    [[nodiscard]] ToolboxModel& model() { return *m_model; }
    [[nodiscard]] const ToolboxModel& model() const { return *m_model; }

    // ── Добавление виджета в документ ────────────────────────
    // Вызывается, когда пользователь выбрал элемент тулбокса и указал родителя.
    //   itemType  — тип виджета (Button, Label, Tree…)
    //   parentId  — ID виджета-родителя, в который нужно добавить новый.
    // Возвращает ID созданного виджета, или невалидный ID при ошибке.
    WidgetID addWidget(WidgetType itemType, WidgetID parentId) {
        // Создаём команду добавления виджета.
        auto cmd = std::make_unique<AddWidgetCommand>(m_doc, itemType, parentId);

        // Получаем ID, который будет у создаваемого виджета.
        // Чтобы узнать ID до выполнения команды, мы можем создать временный виджет,
        // но команда AddWidgetCommand сама создаст его внутри execute().
        // Поэтому мы сначала выполняем команду, а потом ищем созданный виджет.
        m_doc.history().execute(std::move(cmd));

        // Ищем только что добавленный виджет: у него самый большой ID среди детей родителя.
        // Это временное решение; в будущем AddWidgetCommand может возвращать ID.
        Widget* parent = m_doc.widgetTree().find(parentId);
        if (!parent || parent->children().empty()) {
            return WidgetID{}; // невалидный ID
        }

        // Ищем ребёнка с максимальным ID (последний добавленный).
        Widget* lastChild = parent->children().back();
        return lastChild->id();
    }

    // ── Двойной щелчок по элементу тулбокса ──────────────────
    // Если у документа есть корень и корень является контейнером,
    // добавляем виджет прямо в корень. Иначе ничего не делаем.
    // Возвращает ID созданного виджета или невалидный ID.
    WidgetID addWidgetToRoot(WidgetType itemType) {
        Widget* root = m_doc.widgetTree().root();
        if (!root) {
            return WidgetID{};
        }
        return addWidget(itemType, root->id());
    }

    // ── Поиск элемента тулбокса по типу виджета ──────────────
    [[nodiscard]] const ToolboxItem* findItemByType(WidgetType type) const {
        return m_model->findByType(type);
    }

private:
    UIDocument& m_doc;
    std::unique_ptr<ToolboxModel> m_model;

    // Заполняет модель стандартными виджетами, доступными в редакторе.
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

} // namespace MirUI