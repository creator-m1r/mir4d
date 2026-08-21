#pragma once

// MirEngine/BRep/Topology/BRepTypes.hpp
//
// Фундаментальные идентификаторы и ориентации BRep.
// Топология и геометрия ссылаются только на эти типы.

#include <cstdint>
#include <limits>

namespace mir
{

using BRepIndex = std::uint32_t;

inline constexpr BRepIndex InvalidBRepIndex = std::numeric_limits<BRepIndex>::max();

[[nodiscard]] constexpr bool isValidBRepIndex(BRepIndex index) noexcept
{
    return index != InvalidBRepIndex;
}

enum class BRepOrientation : std::uint8_t
{
    Forward = 0,
    Reversed = 1,
    Internal = 2,
    External = 3
};

[[nodiscard]] constexpr BRepOrientation reversed(BRepOrientation orientation) noexcept
{
    switch (orientation)
    {
        case BRepOrientation::Forward:  return BRepOrientation::Reversed;
        case BRepOrientation::Reversed: return BRepOrientation::Forward;
        case BRepOrientation::Internal: return BRepOrientation::External;
        case BRepOrientation::External: return BRepOrientation::Internal;
    }
    return BRepOrientation::Forward;
}

[[nodiscard]] constexpr bool isForward(BRepOrientation orientation) noexcept
{
    return orientation == BRepOrientation::Forward;
}

enum class BRepShapeType : std::uint8_t
{
    Vertex = 0,
    Edge,
    Wire,
    Face,
    Shell,
    Solid,
    CompSolid,
    Compound,
    Unknown
};

enum class BRepCurveType : std::uint8_t
{
    Unknown = 0,
    Line,
    Circle,
    Ellipse,
    Arc,
    Bezier,
    BSpline,
    Polyline
};

enum class BRepSurfaceType : std::uint8_t
{
    Unknown = 0,
    Plane,
    Cylinder,
    Cone,
    Sphere,
    Torus,
    Bezier,
    BSpline,
    Extrusion,
    Revolution
};

struct BRepTolerance
{
    double linear{1.0e-7};
    double angular{1.0e-9};
    double parametric{1.0e-9};

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return linear > 0.0 && angular > 0.0 && parametric > 0.0;
    }
};

inline constexpr BRepTolerance DefaultBRepTolerance{};

} // namespace mir
