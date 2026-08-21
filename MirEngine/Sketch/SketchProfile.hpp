#pragma once

#include <cstdint>
#include <vector>

namespace mir
{

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

}
