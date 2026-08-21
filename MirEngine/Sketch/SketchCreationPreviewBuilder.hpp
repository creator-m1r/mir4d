#pragma once

#include "SketchGeometry.hpp"

#include <cmath>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace mir
{

enum class SketchCreationTool : std::uint8_t
{
    Line,
    Circle,
    Arc
};

struct SketchCreationPoint
{
    SketchPoint2D point{};
    std::optional<std::uint32_t> snappedGeometryId;
};

struct SketchCreationPreview
{
    SketchCreationTool tool{SketchCreationTool::Line};
    std::vector<SketchPoint2D> points;
    std::optional<SketchPoint2D> snappedPoint;
};

class SketchCreationPreviewBuilder
{
public:
    [[nodiscard]] static SketchCreationPreview line(
        SketchPoint2D start,
        SketchCreationPoint current)
    {
        return {SketchCreationTool::Line, {start, current.point}, current.snappedGeometryId
                    ? std::optional<SketchPoint2D>(current.point)
                    : std::nullopt};
    }

    [[nodiscard]] static SketchCreationPreview circle(
        SketchPoint2D center,
        SketchCreationPoint current)
    {
        return {SketchCreationTool::Circle, {center, current.point}, current.snappedGeometryId
                    ? std::optional<SketchPoint2D>(current.point)
                    : std::nullopt};
    }

    [[nodiscard]] static SketchCreationPreview arc(
        SketchPoint2D center,
        SketchPoint2D start,
        SketchCreationPoint current)
    {
        return {SketchCreationTool::Arc,
                {center, start, current.point},
                current.snappedGeometryId
                    ? std::optional<SketchPoint2D>(current.point)
                    : std::nullopt};
    }
};

}
