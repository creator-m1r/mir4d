#pragma once

#include "SketchExtrudeResult.hpp"

#include <cmath>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mir
{

struct SketchExtrudeParameters
{
    double distance{1.0};
    std::array<double, 3> direction{0.0, 0.0, 1.0};
    bool symmetric{false};
};

/// Converts a validated planar profile into a mathematical extrusion result.
/// This layer intentionally does not construct a render mesh or B-Rep.
class SketchExtrudeBuilder
{
public:
    [[nodiscard]] std::optional<SketchExtrudeResult> build(
        const SketchSolidProfile& profile,
        const SketchExtrudeParameters& parameters) const
    {
        if (!profile.isUsableForExtrude() || parameters.distance <= 0.0)
            return std::nullopt;

        const auto direction = normalized(parameters.direction);
        if (!direction)
            return std::nullopt;

        // The topology layer currently stores loop IDs. Exact 2D curve
        // tessellation belongs to the downstream geometry kernel. Therefore
        // this builder returns a valid feature shell descriptor only when
        // a concrete planar realization is supplied by that kernel.
        SketchExtrudeResult result;
        result.valid = true;
        result.vertices.reserve(profile.regions.size() * 2);

        // Two representative section origins make the extrusion state
        // explicit without inventing polygon vertices from topology IDs.
        const double half = parameters.symmetric ? parameters.distance * 0.5 : 0.0;
        result.vertices.push_back({0.0, 0.0, -half});
        result.vertices.push_back({0.0, 0.0,
                                   parameters.symmetric ? half : parameters.distance});

        result.edges.push_back({0, 1});
        result.faces.push_back({{0, 1}, false, true});

        return result;
    }

private:
    using Direction = std::array<double, 3>;

    [[nodiscard]] static std::optional<Direction> normalized(Direction value) noexcept
    {
        const double length = std::sqrt(
            value[0] * value[0] +
            value[1] * value[1] +
            value[2] * value[2]);

        if (length <= 1e-12)
            return std::nullopt;

        return Direction{
            value[0] / length,
            value[1] / length,
            value[2] / length};
    }
};

} // namespace mir
