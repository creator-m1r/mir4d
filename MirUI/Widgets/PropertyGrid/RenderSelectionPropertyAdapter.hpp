#pragma once

#include "Property.hpp"
#include "../../../MirEngine/Render/Selection/RenderSelectionProperties.hpp"
#include "../../../MirEngine/Render/Selection/RenderSelectionPropertiesFormatter.hpp"

#include <vector>

namespace MirUI
{

class RenderSelectionPropertyAdapter
{
public:
    [[nodiscard]] static std::vector<Property> makeProperties(
        const mir::RenderSelectionProperties& selectionProperties);
};

} // namespace MirUI
