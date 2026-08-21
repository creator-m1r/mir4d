#pragma once

#include "SketchEquation.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

namespace mir
{

struct SketchRectangleScenario
{
    static constexpr std::size_t VariableCount = 8;

    static SketchEquationSystem equations(
        double width,
        double height,
        std::uint32_t firstId = 1) 
    {
        SketchEquationSystem system;
        std::uint32_t id = firstId;

        system.add({id++, SketchConstraintType::Horizontal, {1, 2},
            [](const std::vector<double>& v) { return v[3] - v[1]; }});
        system.add({id++, SketchConstraintType::Vertical, {2, 3},
            [](const std::vector<double>& v) { return v[4] - v[2]; }});
        system.add({id++, SketchConstraintType::Horizontal, {3, 4},
            [](const std::vector<double>& v) { return v[7] - v[5]; }});
        system.add({id++, SketchConstraintType::Vertical, {4, 1},
            [](const std::vector<double>& v) { return v[0] - v[6]; }});

        system.add({id++, SketchConstraintType::Distance, {1, 2},
            [width](const std::vector<double>& v) {
                return (v[2] - v[0]) - width;
            }});
        system.add({id++, SketchConstraintType::Distance, {2, 3},
            [height](const std::vector<double>& v) {
                return (v[5] - v[3]) - height;
            }});

        system.add({id++, SketchConstraintType::Distance, {1},
            [](const std::vector<double>& v) { return v[0]; }});
        system.add({id++, SketchConstraintType::Distance, {1},
            [](const std::vector<double>& v) { return v[1]; }});

        return system;
    }
};

}
