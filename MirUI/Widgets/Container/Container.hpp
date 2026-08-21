
#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetType.hpp"
#include "../../Schema/WidgetSchema.hpp"
#include <vector>
#include <algorithm>

namespace MirUI {

class Container : public Widget {
public:

    explicit Container(WidgetType type = WidgetType::Panel)
        : Widget(type)
    {

        setLayoutData(LayoutData::fit());
    }

    bool addChild(Widget* child) {
        if (!child) return false;

        if (!canAcceptChildType(child->type())) {
            return false;
        }

        Widget::addChild(child);
        return true;
    }

    bool removeChild(WidgetID id) {
        return Widget::removeChild(id);
    }

    [[nodiscard]] bool acceptsChildren() const {
        return true;
    }

    [[nodiscard]] bool canAcceptChildType(WidgetType childType) const {
        return WidgetSchema::canContain(this->type(), childType);
    }

    void removeAllChildren() {
        while (!children().empty()) {
            removeChild(children().back()->id());
        }
    }

    [[nodiscard]] size_t childCount() const {
        return children().size();
    }

    [[nodiscard]] LayoutDirection layoutDirection() const {
        auto val = getProperty("layoutDirection");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            const std::string& dir = std::get<std::string>(*val);
            if (dir == "horizontal") return LayoutDirection::Horizontal;
        }
        return LayoutDirection::Vertical;
    }

    void setLayoutDirection(LayoutDirection dir) {
        setProperty("layoutDirection",
            StateValue(std::string(dir == LayoutDirection::Horizontal ? "horizontal" : "vertical")));
    }

    [[nodiscard]] Insets padding() const {
        return Insets{
            getDoubleProperty("paddingTop", 0.0),
            getDoubleProperty("paddingRight", 0.0),
            getDoubleProperty("paddingBottom", 0.0),
            getDoubleProperty("paddingLeft", 0.0)
        };
    }

    void setPadding(const Insets& insets) {
        setProperty("paddingTop",    StateValue(insets.top));
        setProperty("paddingRight",  StateValue(insets.right));
        setProperty("paddingBottom", StateValue(insets.bottom));
        setProperty("paddingLeft",   StateValue(insets.left));
    }

    [[nodiscard]] double spacing() const {
        return getDoubleProperty("spacing", 4.0);
    }

    void setSpacing(double spacing) {
        setProperty("spacing", StateValue(spacing));
    }

private:

    double getDoubleProperty(const std::string& name, double defaultValue) const {
        auto val = getProperty(name);
        if (val.has_value()) {
            if (std::holds_alternative<double>(*val))
                return std::get<double>(*val);
            if (std::holds_alternative<int64_t>(*val))
                return static_cast<double>(std::get<int64_t>(*val));
        }
        return defaultValue;
    }
};

}