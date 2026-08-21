
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "../../Core/Widget/WidgetType.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Foundation/Icons/IconID.hpp"
#include "../Inspector/PropertyDescriptor.hpp"

namespace MirUI {

struct WidgetDescriptor {
    WidgetType type;
    std::string name;
    std::string icon;
    std::function<std::unique_ptr<Widget>()> factory;

    std::vector<PropertyDescriptor> properties;

    bool isContainer = false;
    std::vector<WidgetType> allowedChildren;

    WidgetDescriptor() = default;

    WidgetDescriptor(WidgetType type,
                     std::string name,
                     std::string icon,
                     std::function<std::unique_ptr<Widget>()> factory)
        : type(type)
        , name(std::move(name))
        , icon(std::move(icon))
        , factory(std::move(factory))
    {}

    WidgetDescriptor(WidgetType type,
                     std::string name,
                     std::string icon,
                     std::function<std::unique_ptr<Widget>()> factory,
                     std::vector<PropertyDescriptor> properties,
                     bool isContainer = false,
                     std::vector<WidgetType> allowedChildren = {})
        : type(type)
        , name(std::move(name))
        , icon(std::move(icon))
        , factory(std::move(factory))
        , properties(std::move(properties))
        , isContainer(isContainer)
        , allowedChildren(std::move(allowedChildren))
    {}

    [[nodiscard]] std::unique_ptr<Widget> create() const {
        if (factory) {
            return factory();
        }
        return nullptr;
    }
};

}