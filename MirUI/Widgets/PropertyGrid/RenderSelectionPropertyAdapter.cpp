#include "RenderSelectionPropertyAdapter.hpp"

#include <utility>

namespace MirUI
{

std::vector<Property> RenderSelectionPropertyAdapter::makeProperties(
    const MirEngine::Rendering::RenderSelectionProperties& selectionProperties)
{
    std::vector<Property> properties;

    const auto rows = MirEngine::Rendering::RenderSelectionPropertiesFormatter::format(selectionProperties);
    properties.reserve(rows.size());

    for (const auto& row : rows)
    {
        Property property;
        property.id = "selection." + row.name;
        property.name = row.name;
        property.category = "Выделение";
        property.value = row.value;
        property.readOnly = true;
        properties.push_back(std::move(property));
    }

    return properties;
}

} // namespace MirUI
