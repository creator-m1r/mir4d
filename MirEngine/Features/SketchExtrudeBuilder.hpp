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

        SketchExtrudeResult result;
        result.valid = true;
        result.vertices.reserve(profile.regions.size() * 2);

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

}
