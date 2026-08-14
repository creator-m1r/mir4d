#pragma once

#include "RenderSelectionProperties.hpp"

#include <string>
#include <vector>

namespace MirEngine {
namespace Rendering {

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

} // namespace Rendering
} // namespace MirEngine
