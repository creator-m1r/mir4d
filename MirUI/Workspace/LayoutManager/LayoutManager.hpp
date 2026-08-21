// MirUI/Workspace/LayoutManager/LayoutManager.hpp
// Менеджер компоновки рабочего пространства.
// Управляет расположением виджетов (панелей, viewport'ов) внутри окон,
// позволяет стыковать, разделять, изменять размеры и делать плавающие панели.
// Использует наш универсальный LayoutEngine для расчёта геометрии.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Layout/LayoutEngine.hpp"
#include "../../Core/Layout/LayoutNode.hpp"
#include "../../Core/Widget/WidgetTree.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <memory>
#include <vector>
#include <string>

namespace MirUI {

class LayoutManager {
public:
    // ── Конструктор ──────────────────────────────────────────

    // При создании LayoutManager получает ссылку на дерево виджетов,
    // с которым будет работать.
    explicit LayoutManager(WidgetTree& tree)
        : m_tree(tree)
    {
        // Создаём движок компоновки по умолчанию.
        // Позже можно будет заменить на более продвинутый (с учётом Dock, Grid и т.д.).
        m_layoutEngine = std::make_unique<LayoutEngine>();
    }

    // ── Основные операции компоновки ─────────────────────────

    // Пристыковать (dock) один виджет к другому.
    // Например, пристыковать панель "Инспектор" к правой стороне окна.
    void dock(WidgetID widgetToDock, WidgetID targetWidget, DockPosition position) {
        // Находим оба виджета в дереве.
        Widget* source = m_tree.find(widgetToDock);
        Widget* target = m_tree.find(targetWidget);
        if (!source || !target) return;

        // Удаляем source из его текущего родителя, если он есть.
        Widget* oldParent = source->parent();
        if (oldParent) {
            oldParent->removeChild(widgetToDock);
        }

        // Создаём контейнер-обёртку, который будет управлять расположением.
        // В будущем здесь должен быть специализированный контейнер (например, DockContainer).
        // Пока упростим: создадим обычный виджет-контейнер, который просто добавит source
        // как дочерний к целевому виджету с учётом позиции.
        // Реальное позиционирование (слева/справа/снизу/сверху) будет обрабатываться
        // более сложным LayoutEngine позже. Сейчас просто добавляем как дочерний.
        target->addChild(source);
        
        // TODO: В будущем мы будем заменять target на специальный SplitContainer,
        // который знает, как расположить две панели рядом или друг над другом.
    }

    // Отстыковать виджет (сделать плавающим) — удаляет виджет из текущего родителя
    // и, возможно, помещает в отдельное окно. Пока просто удаляет из родителя.
    void undock(WidgetID widgetID) {
        Widget* widget = m_tree.find(widgetID);
        if (!widget) return;

        Widget* parent = widget->parent();
        if (parent) {
            parent->removeChild(widgetID);
            // TODO: Здесь мы должны были бы создать для виджета новое окно.
            // В будущем WindowManager будет отвечать за создание окон.
        }
    }

    // Разделить область пополам по горизонтали (слева/справа).
    // Пока заглушка — в будущем будет создавать SplitContainer.
    void splitHorizontal(WidgetID containerID, WidgetID newWidget) {
        // TODO: Реализовать настоящее разделение, когда появится поддержка
        // SplitContainer и более продвинутый LayoutEngine.
    }

    // Разделить область пополам по вертикали (сверху/снизу).
    void splitVertical(WidgetID containerID, WidgetID newWidget) {
        // TODO: Аналогично splitHorizontal.
    }

    // Изменить размер виджета. Пока просто устанавливает новые bounds,
    // в будущем учтёт ограничения и соседние виджеты.
    void resize(WidgetID widgetID, const Size& newSize) {
        Widget* widget = m_tree.find(widgetID);
        if (!widget) return;

        // Получаем текущие границы и меняем только ширину и высоту.
        Rect bounds = widget->bounds();
        bounds.width  = newSize.width;
        bounds.height = newSize.height;
        widget->setBounds(bounds);

        // Запускаем перерасчёт компоновки, так как изменение размера одного виджета
        // может повлиять на соседей (в будущем).
        m_layoutEngine->layout(m_tree);
    }

    // Сделать панель плавающей (откреплённой) — пока просто удаляет из родителя.
    void floatPanel(WidgetID panelID) {
        // То же самое, что и undock.
        undock(panelID);
        // TODO: Создать плавающее окно через WindowManager.
    }

    // Восстановить стандартное расположение для заданного рабочего пространства.
    void restore(const std::string& workspaceName) {
        // Заглушка: в будущем будет загружать сохранённую конфигурацию
        // для указанного рабочего пространства и применять её к дереву виджетов.
        // Сейчас просто запускаем принудительный layout.
        m_layoutEngine->layout(m_tree);
    }

    // ── Доступ к движку компоновки ───────────────────────────

    // Позволяет заменить стандартный LayoutEngine на более продвинутый.
    void setLayoutEngine(std::unique_ptr<LayoutEngine> engine) {
        m_layoutEngine = std::move(engine);
    }

    // Возвращает ссылку на текущий LayoutEngine.
    [[nodiscard]] LayoutEngine* layoutEngine() const {
        return m_layoutEngine.get();
    }

private:
    WidgetTree& m_tree;                           // Дерево виджетов, которым мы управляем.
    std::unique_ptr<LayoutEngine> m_layoutEngine; // Движок для расчёта геометрии.
};

} // namespace MirUI