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

}
