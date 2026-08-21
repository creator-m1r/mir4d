#pragma once

#include "Property.hpp"
#include "../../../MirEngine/Rendering/Selection/RenderSelectionProperties.hpp"
#include "../../../MirEngine/Rendering/Selection/RenderSelectionPropertiesFormatter.hpp"

#include <vector>

namespace MirUI
{

class RenderSelectionPropertyAdapter
{
public:
    [[nodiscard]] static std::vector<Property> makeProperties(
        const MirEngine::Rendering::RenderSelectionProperties& selectionProperties);
};

}
