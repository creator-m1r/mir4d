#pragma once

#include <cstddef>
#include <cstdint>

namespace mir
{

enum class SketchSolveStatus : std::uint8_t
{
    Solved,
    UnderConstrained,
    OverConstrained,
    Inconsistent,
    Failed,
    Invalid
};

struct SketchSolveDiagnostics
{
    SketchSolveStatus status{SketchSolveStatus::Invalid};
    std::size_t degreesOfFreedom{0};
    std::size_t variableCount{0};
    std::size_t equationCount{0};
    std::size_t invalidEquationCount{0};
    std::size_t iterations{0};
    double residual{0.0};
    double maximumResidual{0.0};
};

} // namespace mir
