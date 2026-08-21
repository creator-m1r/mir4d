#pragma once

#include "RenderSelection.hpp"

#include <cstddef>
#include <cstdint>

namespace MirEngine {
namespace Rendering {

struct RenderVec3
{
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

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

}
}