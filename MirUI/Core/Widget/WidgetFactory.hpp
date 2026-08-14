// MirUI/Core/Widget/WidgetFactory.hpp
#pragma once

#include "Widget.hpp"
#include "WidgetType.hpp"
#include "WidgetTree.hpp"
#include "../../Widgets/Button/Button.hpp"
#include "../../Widgets/Label/Label.hpp"
#include "../../Widgets/Tree/Tree.hpp"
#include "../../Widgets/PropertyGrid/PropertyGrid.hpp"
#include "../../Widgets/Viewport/Viewport.hpp"
#include "../../Widgets/Toolbar/Toolbar.hpp"
#include "../../Widgets/Container/Container.hpp"
#include "../../Widgets/TextField/TextField.hpp"
#include "../../Widgets/CheckBox/CheckBox.hpp"
#include "../../Widgets/Slider/Slider.hpp"
#include <memory>
#include <string>

namespace MirUI {

class WidgetFactory {
public:
    // Создаёт виджет нужного типа. Владение передаётся вызывающему.
    // Регистрацию в WidgetTree (setRoot / addChild + registerWidget)
    // выполняет вызывающий код.
    static std::unique_ptr<Widget> create(WidgetType type, const std::string& name = "") {
        std::unique_ptr<Widget> widget;

        switch (type) {
            case WidgetType::Button:       widget = std::make_unique<Button>(); break;
            case WidgetType::Label:        widget = std::make_unique<Label>(); break;
            case WidgetType::Tree:         widget = std::make_unique<Tree>(); break;
            case WidgetType::PropertyGrid: widget = std::make_unique<PropertyGrid>(); break;
            case WidgetType::Viewport:     widget = std::make_unique<Viewport>(); break;
            case WidgetType::Toolbar:      widget = std::make_unique<Toolbar>(); break;
            case WidgetType::Panel:
            case WidgetType::DockPanel:
            case WidgetType::Container:
            case WidgetType::Window:       widget = std::make_unique<Container>(type); break;
            case WidgetType::TextField:    widget = std::make_unique<TextField>(); break;
            case WidgetType::CheckBox:     widget = std::make_unique<CheckBox>(); break;
            case WidgetType::Slider:       widget = std::make_unique<Slider>(); break;
            default:                       widget = std::make_unique<Widget>(type); break;
        }

        if (!name.empty()) {
            widget->setName(name);
        }

        return widget;
    }
};

} // namespace MirUI