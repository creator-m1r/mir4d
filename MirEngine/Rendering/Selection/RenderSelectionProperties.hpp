#pragma once

#include "RenderSelection.hpp"
#include "../Resources/RenderMesh.hpp"
#include "../Camera/RenderTypes.hpp"

#include <cstddef>
#include <cstdint>

namespace MirEngine {
namespace Rendering {

/// Render-layer selection properties.
/// Uses the canonical lightweight render vector shared by the render subsystem.
struct RenderSelectionProperties
{
    RenderSelection selection{};
    std::size_t triangleCount{0};
    double area{0.0};
    RenderVec3 center{};
    RenderVec3 normal{};

    [[nodiscard]] bool valid() const noexcept
    {
        return selection.valid();
    }
};

class RenderSelectionPropertiesBuilder
{
public:
    [[nodiscard]] static RenderSelectionProperties build(
        const RenderSelection& selection,
        const RenderMesh& mesh) noexcept;
};

} // namespace Rendering
} // namespace MirEngine
