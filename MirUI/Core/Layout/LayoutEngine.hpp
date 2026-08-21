
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

    void layout(WidgetTree& tree) {
        Widget* root = tree.root();
        if (!root) return;

        Rect rootBounds = root->bounds();
        if (rootBounds.width <= 0 || rootBounds.height <= 0) {
            rootBounds = calculateWidgetSize(root, Rect{0,0,1920,1080});
            root->setBounds(rootBounds);
        }

        layoutChildren(*root);
    }

protected:

    Rect calculateWidgetSize(Widget* widget, const Rect& parentBounds) {
        if (!widget) return Rect{0,0,0,0};

        const LayoutData& ld = widget->layoutData();
        double w = 0.0, h = 0.0;

        switch (ld.widthUnit) {
        case Unit::Pixel:
            w = ld.widthValue;
            break;
        case Unit::Percent:
            w = parentBounds.width * (ld.widthValue / 100.0);
            break;
        case Unit::Auto:

            if (ld.horizontalPolicy == SizePolicy::Fill) {
                w = parentBounds.width;
            } else {

                w = calculateChildrenMaxWidth(widget);
                if (w <= 0) w = parentBounds.width * 0.5;
            }
            break;
        }

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

                h = calculateChildrenTotalHeight(widget);
                if (h <= 0) h = parentBounds.height * 0.5;
            }
            break;
        }

        w = std::clamp(w, ld.minimumSize.width, ld.maximumSize.width);
        h = std::clamp(h, ld.minimumSize.height, ld.maximumSize.height);

        return Rect{0, 0, w, h};
    }

    virtual void layoutChildren(Widget& parent) {
        const auto& children = parent.children();
        if (children.empty()) return;

        Rect parentBounds = parent.bounds();
        if (parentBounds.width <= 0 || parentBounds.height <= 0) return;

        double currentY = parentBounds.y;
        double availableHeight = parentBounds.height;
        double totalFixedHeight = 0.0;
        int fillCount = 0;

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

        double heightForFills = std::max(0.0, availableHeight - totalFixedHeight);
        double fillHeightEach = (fillCount > 0) ? heightForFills / fillCount : 0.0;

        for (auto& info : infos) {
            double finalHeight = info.desiredHeight;
            if (info.vpol == SizePolicy::Fill) {
                finalHeight = fillHeightEach;
            }

            const LayoutData& ld = info.widget->layoutData();
            finalHeight = std::clamp(finalHeight, ld.minimumSize.height, ld.maximumSize.height);

            double childWidth = parentBounds.width;
            if (ld.horizontalPolicy == SizePolicy::Fixed || ld.widthUnit == Unit::Pixel) {
                childWidth = calculateWidgetSize(info.widget, parentBounds).width;
            }
            childWidth = std::clamp(childWidth, ld.minimumSize.width, ld.maximumSize.width);

            Rect childRect(parentBounds.x, currentY, childWidth, finalHeight);
            info.widget->setBounds(childRect);

            layoutChildren(*info.widget);

            currentY += finalHeight;
        }
    }

    double calculateChildrenMaxWidth(Widget* parent) {
        double maxWidth = 0.0;
        for (Widget* child : parent->children()) {
            if (!child->isVisible()) continue;
            Rect childSize = calculateWidgetSize(child, parent->bounds());
            maxWidth = std::max(maxWidth, childSize.width);
        }
        return maxWidth;
    }

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

}