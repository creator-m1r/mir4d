#pragma once

#include "SketchExtrudeGeometry.hpp"
#include "../Sketch/SketchSolidProfile.hpp"

#include <cmath>
#include <cstdint>

namespace mir
{

struct SketchExtrudeParameters
{
    double distance{1.0};
    SketchExtrudeVector3D direction{};
    bool symmetric{false};

    [[nodiscard]] bool valid() const noexcept
    {
        const double lengthSquared = direction.lengthSquared();
        return std::isfinite(distance) && distance > 1e-9 &&
               std::isfinite(lengthSquared) && lengthSquared > 1e-24;
    }
};

/// Mathematical description of an extrusion feature.
/// It deliberately does not generate a mesh or B-Rep yet.
struct SketchExtrudeFeature
{
    std::uint32_t id{0};
    SketchSolidProfile profile;
    SketchExtrudeParameters parameters;

    [[nodiscard]] bool valid() const noexcept
    {
        return profile.valid && parameters.valid();
    }

    [[nodiscard]] double startOffset() const noexcept
    {
        return parameters.symmetric ? -parameters.distance * 0.5 : 0.0;
    }

    [[nodiscard]] double endOffset() const noexcept
    {
        return parameters.symmetric ? parameters.distance * 0.5 : parameters.distance;
    }
};

} // namespace mir
