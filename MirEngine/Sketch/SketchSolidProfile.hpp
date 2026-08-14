#pragma once

#include "SketchProfileLoops.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mir
{

struct SketchSolidRegion
{
    std::size_t outerLoop{0};
    std::vector<std::size_t> holes;
};

/// Immutable-ready description of planar material regions for a 3D feature.
/// It contains topology only; triangulation and B-Rep construction belong to
/// the downstream solid kernel.
struct SketchSolidProfile
{
    std::uint32_t id{0};
    std::vector<SketchSolidRegion> regions;
    bool valid{false};

    [[nodiscard]] bool isUsableForExtrude() const noexcept
    {
        return valid && !regions.empty();
    }
};

} // namespace mir
