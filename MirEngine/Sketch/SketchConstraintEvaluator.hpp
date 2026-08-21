#pragma once

#include "SketchEquation.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace mir
{

struct SketchResidualReport
{
    double squaredError{0.0};
    double maximumError{0.0};
    std::size_t equationCount{0};
    std::size_t invalidEquationCount{0};
};

class SketchConstraintEvaluator
{
public:
    [[nodiscard]] SketchResidualReport evaluate(
        const SketchEquationSystem& system,
        const std::vector<double>& variables) const noexcept
    {
        SketchResidualReport report{};
        report.equationCount = system.all().size();

        for (const auto& equation : system.all())
        {
            if (!equation.residual)
            {
                ++report.invalidEquationCount;
                continue;
            }

            const double residual = equation.residual(variables);
            if (!std::isfinite(residual))
            {
                ++report.invalidEquationCount;
                continue;
            }

            report.squaredError += residual * residual;
            report.maximumError = std::max(report.maximumError, std::abs(residual));
        }

        return report;
    }
};

} // namespace mir
