// MirUI/Foundation/Animation/AnimationManager.hpp
// Animation engine contract for MirUI (DesignerCore and UIContext).
//
// This file is a placeholder: it keeps the documented API
// (setWidgetTree / update / animate / stopWidget) available and compilable.
// The actual animation evaluation engine is a separate milestone and is
// intentionally not implemented here yet.

#pragma once

#include "Animation.hpp"
#include "AnimationSpec.hpp"
#include <string>

namespace MirUI {

class Widget;      // defined in Core/Widget/Widget.hpp
class WidgetTree;  // defined in Core/Widget/WidgetTree.hpp
class WidgetID;    // defined in Core/Widget/WidgetID.hpp

class AnimationManager {
public:
    AnimationManager() = default;

    void setWidgetTree(WidgetTree* tree) noexcept { m_widgetTree = tree; }

    // Advances all active animations by deltaTime seconds.
    void update(double deltaTime) noexcept { (void)deltaTime; }

    // Stops any running animation of the given widget.
    void stopWidget(WidgetID widgetId) noexcept { (void)widgetId; }

    // Registers an animated property change. The value type is caller
    // defined (StateValue in MirUI Designer); the engine that evaluates
    // these transitions is a separate milestone.
    template <typename ValueT>
    void animate(Widget& widget, const std::string& propertyName,
                 const ValueT& targetValue, const AnimationSpec& spec) noexcept {
        (void)widget; (void)propertyName; (void)targetValue; (void)spec;
    }

private:
    WidgetTree* m_widgetTree = nullptr;
};

} // namespace MirUI
