#pragma once

#include "SketchRectangleScenario.hpp"
#include "SketchSolverNewton.hpp"

#include <array>
#include <cstddef>

namespace mir
{

struct SketchRectangleSolveOutput
{
    SketchNewtonResult solver{};
    std::array<double, SketchRectangleScenario::VariableCount> variables{};
};

class SketchRectangleSolver
{
public:
    [[nodiscard]] SketchRectangleSolveOutput solve(
        double width,
        double height,
        std::array<double, SketchRectangleScenario::VariableCount> initial = {}) const
    {
        SketchRectangleSolveOutput output{SketchNewtonResult{}, initial};
        auto equations = SketchRectangleScenario::equations(width, height);
        std::vector<double> variables(output.variables.begin(), output.variables.end());

        output.solver = SketchSolverNewton{}.solve(equations, variables);
        for (std::size_t i = 0; i < output.variables.size(); ++i)
            output.variables[i] = variables[i];

        return output;
    }
};

}
