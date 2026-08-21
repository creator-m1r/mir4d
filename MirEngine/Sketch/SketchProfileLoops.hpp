#pragma once

#include "SketchGeometry.hpp"
#include "SketchProfile.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace mir
{

struct SketchProfileLoop
{
    std::vector<std::uint32_t> geometryIDs;
    double signedArea{0.0};
    bool closed{false};
    bool valid{false};

    [[nodiscard]] bool isHole() const noexcept { return signedArea < 0.0; }
};

struct SketchProfileLoops
{
    std::vector<SketchProfileLoop> loops;
    std::optional<std::size_t> outerLoopIndex;

    [[nodiscard]] bool valid() const noexcept
    {
        return outerLoopIndex.has_value() && !loops.empty() &&
               std::all_of(loops.begin(), loops.end(), [](const auto& loop) {
                   return loop.closed && loop.valid && std::abs(loop.signedArea) > 1e-9;
               });
    }

    [[nodiscard]] const SketchProfileLoop* outer() const noexcept
    {
        if (!outerLoopIndex || *outerLoopIndex >= loops.size())
            return nullptr;
        return &loops[*outerLoopIndex];
    }
};

} // namespace mir
