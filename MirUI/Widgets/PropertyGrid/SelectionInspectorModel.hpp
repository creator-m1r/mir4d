#pragma once

#include "Property.hpp"
#include "RenderSelectionPropertyAdapter.hpp"

#include <vector>

namespace MirUI
{

class SelectionInspectorModel
{
public:
    void clear() noexcept
    {
        properties_.clear();
    }

    void update(const MirEngine::Rendering::RenderSelectionProperties& selectionProperties)
    {
        properties_ = RenderSelectionPropertyAdapter::makeProperties(selectionProperties);
    }

    [[nodiscard]] const std::vector<Property>& properties() const noexcept
    {
        return properties_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return properties_.empty();
    }

private:
    std::vector<Property> properties_{};
};

} // namespace MirUI
