#pragma once

#include "SketchProfile.hpp"
#include "SketchRectangleScenario.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

namespace mir
{

struct SketchRectangleProfileBuilder
{
    static SketchProfile build(
        const std::vector<double>& variables,
        std::uint32_t profileId = 1,
        std::uint32_t firstGeometryId = 1,
        double tolerance = 1.0e-8)
    {
        SketchProfile profile;
        profile.id = profileId;

        if (variables.size() != SketchRectangleScenario::VariableCount)
            return profile;

        profile.geometryIDs = {
            firstGeometryId,
            firstGeometryId + 1,
            firstGeometryId + 2,
            firstGeometryId + 3
        };

        const auto close = [tolerance](double a, double b) {
            return std::abs(a - b) <= tolerance;
        };

        const double x1 = variables[0];
        const double y1 = variables[1];
        const double x2 = variables[2];
        const double y2 = variables[3];
        const double x3 = variables[4];
        const double y3 = variables[5];
        const double x4 = variables[6];
        const double y4 = variables[7];

        const bool horizontal12 = close(y1, y2);
        const bool vertical23 = close(x2, x3);
        const bool horizontal34 = close(y3, y4);
        const bool vertical41 = close(x4, x1);

        const double edge12 = std::hypot(x2 - x1, y2 - y1);
        const double edge23 = std::hypot(x3 - x2, y3 - y2);
        const double edge34 = std::hypot(x4 - x3, y4 - y3);
        const double edge41 = std::hypot(x1 - x4, y1 - y4);

        profile.signedArea = 0.5 *
            (x1 * y2 + x2 * y3 + x3 * y4 + x4 * y1 -
             y1 * x2 - y2 * x3 - y3 * x4 - y4 * x1);

        const bool nonDegenerate =
            edge12 > tolerance && edge23 > tolerance &&
            edge34 > tolerance && edge41 > tolerance;

        profile.closed = horizontal12 && vertical23 &&
                         horizontal34 && vertical41 && nonDegenerate;
        profile.selfIntersecting = false;
        profile.valid = profile.closed &&
                        std::abs(profile.signedArea) > tolerance;

        return profile;
    }
};

}
