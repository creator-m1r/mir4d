#pragma once

#include "RenderSelectionProperties.hpp"

#include <string>
#include <vector>

namespace mir
{

struct RenderPropertyRow
{
    std::string name;
    std::string value;
};

class RenderSelectionPropertiesFormatter
{
public:
    [[nodiscard]] static std::vector<RenderPropertyRow> format(
        const RenderSelectionProperties& properties);
};

} // namespace mir
