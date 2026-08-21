#pragma once

#include "SketchGeometryEditCommand.hpp"
#include "SketchGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>

namespace mir
{

struct SketchManipulationResult
{
    SketchGeometry geometry;
    bool changed{false};
};

/// Computes a proposed geometry state from a selected handle.
/// It does not mutate the document; the caller can wrap the result in
/// SketchGeometryEditCommand and commit it atomically.
class SketchGeometryManipulator
{
public:
    [[nodiscard]] static std::optional<SketchManipulationResult> move(
        const SketchGeometry& source,
        SketchGeometryHandle handle,
        SketchPoint2D point) noexcept
    {
        SketchGeometry result = source;

        return std::visit(
            [&](auto& geometry) -> std::optional<SketchManipulationResult>
            {
                using T = std::decay_t<decltype(geometry)>;

                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    if (handle == SketchGeometryHandle::LineStart)
                        geometry.start = point;
                    else if (handle == SketchGeometryHandle::LineEnd)
                        geometry.end = point;
                    else
                        return std::nullopt;
                }
                else if constexpr (std::is_same_v<T, SketchCircle2D>)
                {
                    if (handle == SketchGeometryHandle::CircleCenter)
                    {
                        geometry.center = point;
                    }
                    else if (handle == SketchGeometryHandle::CircleRadius)
                    {
                        const double dx = point.x - geometry.center.x;
                        const double dy = point.y - geometry.center.y;
                        geometry.radius = std::max(1e-9, std::sqrt(dx * dx + dy * dy));
                    }
                    else
                    {
                        return std::nullopt;
                    }
                }
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                {
                    if (handle == SketchGeometryHandle::ArcCenter)
                    {
                        geometry.center = point;
                    }
                    else
                    {
                        return std::nullopt;
                    }
                }

                return SketchManipulationResult{
                    result,
                    result != source};
            },
            result);
    }
};

} // namespace mir
