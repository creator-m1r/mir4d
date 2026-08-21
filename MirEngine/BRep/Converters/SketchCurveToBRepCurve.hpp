#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"
#include "MirEngine/Sketch/SketchProfileGeometryResolver.hpp"

#include <cmath>
#include <optional>

namespace mir
{

class SketchCurveToBRepCurve
{
public:
    [[nodiscard]] static std::optional<BRepCurveGeometry> convert(
        const SketchResolvedCurve& curve,
        Vector3 normal = {0.0, 0.0, 1.0}) noexcept
    {
        BRepCurveGeometry result{};
        result.range = {0.0, 1.0};

        switch (curve.kind)
        {
        case SketchCurveKind::Line:
        {
            const Vector3 start{curve.start.x, curve.start.y, 0.0};
            const Vector3 end{curve.end.x, curve.end.y, 0.0};
            const Vector3 direction = end - start;
            if (direction.isZero())
                return std::nullopt;
            result.type = BRepCurveType::Line;
            result.line.location = start;
            result.line.direction = direction;
            return result;
        }

        case SketchCurveKind::Arc:
            result.type = BRepCurveType::Arc;
            result.range = {curve.startAngle, curve.endAngle};
            result.circle.center = {curve.center.x, curve.center.y, 0.0};
            result.circle.normal = normal;
            result.circle.radius = curve.radius;
            return result.isValid() ? std::optional<BRepCurveGeometry>{result} : std::nullopt;

        case SketchCurveKind::Circle:
            result.type = BRepCurveType::Circle;
            result.range = {0.0, 2.0 * std::acos(-1.0)};
            result.circle.center = {curve.center.x, curve.center.y, 0.0};
            result.circle.normal = normal;
            result.circle.radius = curve.radius;
            return result.isValid() ? std::optional<BRepCurveGeometry>{result} : std::nullopt;
        }

        return std::nullopt;
    }
};

}
