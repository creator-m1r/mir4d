#pragma once

#include "SketchGeometry.hpp"
#include "SketchProfileLoops.hpp"
#include "SketchSolidProfile.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace mir
{

/// Converts validated planar loops into material regions suitable for an
/// extrusion operation. This layer deliberately does not triangulate or
/// create B-Rep topology.
class SketchSolidProfileBuilder
{
public:
    [[nodiscard]] static std::optional<SketchSolidProfile> build(
        std::uint32_t profileID,
        const SketchProfileLoops& loops)
    {
        if (!loops.valid() || !loops.outerLoopIndex)
            return std::nullopt;

        SketchSolidProfile profile;
        profile.id = profileID;

        const std::size_t outer = *loops.outerLoopIndex;
        SketchSolidRegion region;
        region.outerLoop = outer;

        for (std::size_t i = 0; i < loops.loops.size(); ++i)
        {
            if (i == outer)
                continue;

            // The current loop detector provides orientation. Opposite
            // orientation is treated as a hole; nested islands can be
            // promoted to additional regions by a later containment pass.
            if (loops.loops[i].isHole())
                region.holes.push_back(i);
            else
            {
                SketchSolidRegion island;
                island.outerLoop = i;
                profile.regions.push_back(std::move(island));
            }
        }

        profile.regions.insert(profile.regions.begin(), std::move(region));
        profile.valid = !profile.regions.empty();
        return profile;
    }
};

} // namespace mir
