#pragma once

#include "PropertyGrid.hpp"
#include "SelectionInspectorModel.hpp"

namespace MirUI
{

class SelectionPropertiesPanel
{
public:
    void clear() noexcept
    {
        model_.clear();
        grid_.clear();
    }

    void update(const MirEngine::Rendering::RenderSelectionProperties& selectionProperties)
    {
        model_.update(selectionProperties);
        grid_.setProperties(model_.properties());
    }

    [[nodiscard]] const SelectionInspectorModel& model() const noexcept
    {
        return model_;
    }

    [[nodiscard]] const PropertyGrid& grid() const noexcept
    {
        return grid_;
    }

    [[nodiscard]] PropertyGrid& grid() noexcept
    {
        return grid_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return model_.empty();
    }

private:
    SelectionInspectorModel model_{};
    PropertyGrid grid_{};
};

} // namespace MirUI
