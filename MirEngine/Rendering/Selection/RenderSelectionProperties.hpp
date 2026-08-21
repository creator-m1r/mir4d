#pragma once

#include "RenderSelection.hpp"

#include <cstddef>
#include <cstdint>

namespace MirEngine {
namespace Rendering {

/// Canonical lightweight render-space vector shared by the render layer.
/// Deliberately independent of the geometry math namespace so the render
/// layer stays self-contained (AGENTS.md: no C++ <-> Swift-only types).
struct RenderVec3
{
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

/// Render-layer selection properties (face statistics).
/// Produced by the selection inspector pipeline and formatted for the
/// property grid by RenderSelectionPropertiesFormatter.
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

} // namespace Rendering
} // namespace MirEngine