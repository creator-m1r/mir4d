#pragma once

#include <cstdint>
#include <vector>

namespace mir
{

/// A validated planar loop represented by ordered sketch geometry IDs.
/// Construction geometry is never included in a manufacturing profile.
struct SketchProfile
{
    std::uint32_t id{0};
    std::vector<std::uint32_t> geometryIDs;
    bool closed{false};
    bool valid{false};
    bool selfIntersecting{false};
    double signedArea{0.0};

    [[nodiscard]] bool isUsableForSolidFeature() const noexcept
    {
        return closed && valid && !selfIntersecting && geometryIDs.size() >= 2;
    }
};

} // namespace mir
