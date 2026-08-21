#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometryStore.hpp"
#include "MirEngine/Sketch/SketchProfileGeometryResolver.hpp"

#include <cmath>
#include <optional>

namespace mir
{

class SketchExtrudeSurfaceGeometryBuilder
{
public:
    [[nodiscard]] static std::optional<BRepSurfaceHandle> addSideSurface(
        const SketchResolvedCurve& curve,
        const Vector3& direction,
        Scalar distance,
        BRepGeometryStore& store) noexcept
    {
        if (!(distance > 0.0) || !direction.isFinite())
            return std::nullopt;

        const Scalar length = direction.length();
        if (length <= Scalar(1.0e-12))
            return std::nullopt;

        const Vector3 axis = direction / length;

        switch (curve.kind)
        {
        case SketchCurveKind::Line:
        {
            const Vector3 tangent{curve.end.x - curve.start.x,
                                  curve.end.y - curve.start.y,
                                  0.0};
            const Scalar tangentLength = tangent.length();
            if (tangentLength <= Scalar(1.0e-12))
                return std::nullopt;

            BRepSurfaceGeometry surface{};
            surface.type = BRepSurfaceType::Plane;
            surface.uRange = {0.0, tangentLength};
            surface.vRange = {0.0, distance};
            surface.plane.location = {curve.start.x, curve.start.y, 0.0};
            surface.plane.xDir = tangent / tangentLength;
            surface.plane.normal = Vector3::cross(surface.plane.xDir, axis).normalized();
            return surface.isValid()
                ? std::optional<BRepSurfaceHandle>{store.addSurface(std::move(surface))}
                : std::nullopt;
        }

        case SketchCurveKind::Arc:
        case SketchCurveKind::Circle:
        {
            BRepSurfaceGeometry surface{};
            surface.type = BRepSurfaceType::Cylinder;
            surface.uRange = curve.kind == SketchCurveKind::Arc
                ? BRepRange{curve.startAngle, curve.endAngle}
                : BRepRange{0.0, 2.0 * std::acos(-1.0)};
            surface.vRange = {0.0, distance};
            surface.cylinder.location = {curve.center.x, curve.center.y, 0.0};
            surface.cylinder.axis = axis;
            surface.cylinder.xDir = Vector3::unitX();
            surface.cylinder.radius = curve.radius;
            return surface.isValid()
                ? std::optional<BRepSurfaceHandle>{store.addSurface(std::move(surface))}
                : std::nullopt;
        }
        }

        return std::nullopt;
    }
};

}
