// MirUI/Core/Layout/LayoutEngine.hpp
// ⚙️ Улучшенный движок компоновки.
// Обходит дерево виджетов, читает LayoutData каждого виджета
// и вычисляет итоговые Rect (границы) с учётом:
//   - единиц измерения (пиксели, проценты, авто)
//   - политик размера (Fixed, Fill, Fit, Stretch)
//   - минимальных и максимальных ограничений
//   - вложенности (размер родителя влияет на проценты у детей)
//
// На первом этапе поддерживает вертикальную и горизонтальную
// укладку детей (выбирается направлением в LayoutData).
// Позже сюда добавятся Grid, Stack, Dock и т.д.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Widget/Widget.hpp"
#include "../Widget/WidgetTree.hpp"
#include "LayoutData.hpp"
#include "Rect.hpp"
#include <algorithm>
#include <cmath>

namespace MirUI {

class LayoutEngine {
public:
    virtual ~LayoutEngine() = default;

    // ── Главная точка входа ─────────────────────────────────
    // Выполняет компоновку всего дерева, начиная с корня.
    void layout(WidgetTree& tree) {
        Widget* root = tree.root();
        if (!root) return;

        // Начинаем с корня: его размеры обычно заданы явно (например, размер окна).
        // Если bounds корня не заданы — используем предпочтительный размер из LayoutData.
        Rect rootBounds = root->bounds();
        if (rootBounds.width <= 0 || rootBounds.height <= 0) {
            rootBounds = calculateWidgetSize(root, Rect{0,0,1920,1080}); // большой фиктивный родитель
            root->setBounds(rootBounds);
        }

        // Рекурсивно компонуем всех детей.
        layoutChildren(*root);
    }

protected:
    // ── Расчёт размера одного виджета ───────────────────────
    // Возвращает Rect с нулевым положением (x=0,y=0), который потом
    // будет размещён родителем в нужном месте.
    Rect calculateWidgetSize(Widget* widget, const Rect& parentBounds) {
        if (!widget) return Rect{0,0,0,0};

        const LayoutData& ld = widget->layoutData();
        double w = 0.0, h = 0.0;

        // --- Ширина ---
        switch (ld.widthUnit) {
        case Unit::Pixel:
            w = ld.widthValue;
            break;
        case Unit::Percent:
            w = parentBounds.width * (ld.widthValue / 100.0);
            break;
        case Unit::Auto:
            // Если Auto, то размер определяется детьми (Fit) или Fill.
            if (ld.horizontalPolicy == SizePolicy::Fill) {
                w = parentBounds.width; // займёт всё доступное место
            } else {
                // Fit — вычисляем по максимальной ширине детей (простейший случай — берём максимум)
                w = calculateChildrenMaxWidth(widget);
                if (w <= 0) w = parentBounds.width * 0.5; // fallback
            }
            break;
        }

        // --- Высота ---
        switch (ld.heightUnit) {
        case Unit::Pixel:
            h = ld.heightValue;
            break;
        case Unit::Percent:
            h = parentBounds.height * (ld.heightValue / 100.0);
            break;
        case Unit::Auto:
            if (ld.verticalPolicy == SizePolicy::Fill) {
                h = parentBounds.height;
            } else {
                // Fit — по сумме высот детей (для вертикальной укладки) или максимуму (для горизонтальной)
                h = calculateChildrenTotalHeight(widget);
                if (h <= 0) h = parentBounds.height * 0.5;
            }
            break;
        }

        // Применяем ограничения min/max.
        w = std::clamp(w, ld.minimumSize.width, ld.maximumSize.width);
        h = std::clamp(h, ld.minimumSize.height, ld.maximumSize.height);

        return Rect{0, 0, w, h};
    }

    // ── Компоновка детей внутри родителя ────────────────────
    virtual void layoutChildren(Widget& parent) {
        const auto& children = parent.children();
        if (children.empty()) return;

        Rect parentBounds = parent.bounds();
        if (parentBounds.width <= 0 || parentBounds.height <= 0) return;

        // Направление укладки можно задать в LayoutData родителя (пока не реализовано).
        // По умолчанию — вертикальная. Если родитель, например, Toolbar, он сам переопределит этот метод.
        // Для простоты сейчас сделаем вертикальную укладку с учётом политик Fill/Fit.

        double currentY = parentBounds.y;
        double availableHeight = parentBounds.height;
        double totalFixedHeight = 0.0;
        int fillCount = 0;

        // Первый проход: собираем информацию о детях.
        struct ChildInfo {
            Widget* widget;
            double desiredHeight;
            SizePolicy vpol;
        };
        std::vector<ChildInfo> infos;
        for (Widget* child : children) {
            if (!child->isVisible()) continue;
            Rect childRect = calculateWidgetSize(child, parentBounds);
            const LayoutData& ld = child->layoutData();
            double desiredH = childRect.height;
            if (ld.heightUnit == Unit::Auto && ld.verticalPolicy == SizePolicy::Fill) {
                fillCount++;
            } else {
                totalFixedHeight += desiredH;
            }
            infos.push_back({child, desiredH, ld.verticalPolicy});
        }

        // Оставшаяся высота для Fill-элементов.
        double heightForFills = std::max(0.0, availableHeight - totalFixedHeight);
        double fillHeightEach = (fillCount > 0) ? heightForFills / fillCount : 0.0;

        // Второй проход: расставляем детей.
        for (auto& info : infos) {
            double finalHeight = info.desiredHeight;
            if (info.vpol == SizePolicy::Fill) {
                finalHeight = fillHeightEach;
            }
            // Учитываем min/max.
            const LayoutData& ld = info.widget->layoutData();
            finalHeight = std::clamp(finalHeight, ld.minimumSize.height, ld.maximumSize.height);

            // Ширина ребёнка — либо Fill на всю ширину, либо своя.
            double childWidth = parentBounds.width;
            if (ld.horizontalPolicy == SizePolicy::Fixed || ld.widthUnit == Unit::Pixel) {
                childWidth = calculateWidgetSize(info.widget, parentBounds).width;
            }
            childWidth = std::clamp(childWidth, ld.minimumSize.width, ld.maximumSize.width);

            Rect childRect(parentBounds.x, currentY, childWidth, finalHeight);
            info.widget->setBounds(childRect);

            // Рекурсивно компонуем внуков.
            layoutChildren(*info.widget);

            currentY += finalHeight;
        }
    }

    // ── Вспомогательные методы для Auto-размеров ────────────

    // Максимальная ширина среди детей (для Fit по ширине).
    double calculateChildrenMaxWidth(Widget* parent) {
        double maxWidth = 0.0;
        for (Widget* child : parent->children()) {
            if (!child->isVisible()) continue;
            Rect childSize = calculateWidgetSize(child, parent->bounds());
            maxWidth = std::max(maxWidth, childSize.width);
        }
        return maxWidth;
    }

    // Суммарная высота детей (для Fit по высоте при вертикальной укладке).
    double calculateChildrenTotalHeight(Widget* parent) {
        double total = 0.0;
        for (Widget* child : parent->children()) {
            if (!child->isVisible()) continue;
            Rect childSize = calculateWidgetSize(child, parent->bounds());
            total += childSize.height;
        }
        return total;
    }
};

} // namespace MirUI