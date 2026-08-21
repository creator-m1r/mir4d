#pragma once

#include "SketchConstraint.hpp"
#include "SketchGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mir
{

enum class SketchSolveStatus : std::uint8_t
{
    Solved,
    UnderConstrained,
    OverConstrained,
    Invalid
};

struct SketchSolveResult
{
    SketchSolveStatus status{SketchSolveStatus::Invalid};
    std::size_t degreesOfFreedom{0};
    std::size_t geometryCount{0};
    std::size_t constraintCount{0};
};

class SketchSolver
{
public:
    [[nodiscard]] SketchSolveResult analyse(
        const SketchGeometryStore& geometry,
        const SketchConstraintStore& constraints) const noexcept
    {
        SketchSolveResult result{};
        result.geometryCount = geometry.all().size();
        result.constraintCount = constraints.all().size();

        if (result.geometryCount == 0)
        {
            result.status = SketchSolveStatus::Solved;
            return result;
        }

        std::size_t dof = 0;
        for (const auto& item : geometry.all())
        {
            std::visit([&](const auto& value)
            {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, SketchLine2D>)
                    dof += 4;
                else if constexpr (std::is_same_v<T, SketchCircle2D>)
                    dof += 3;
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                    dof += 5;
            }, item);
        }

        const std::size_t effectiveConstraints =
            std::min(dof, constraints.all().size());
        dof -= effectiveConstraints;
        result.degreesOfFreedom = dof;

        if (constraints.all().size() > result.geometryCount * 5 + 8)
            result.status = SketchSolveStatus::OverConstrained;
        else if (dof == 0)
            result.status = SketchSolveStatus::Solved;
        else
            result.status = SketchSolveStatus::UnderConstrained;

        return result;
    }
};

}
