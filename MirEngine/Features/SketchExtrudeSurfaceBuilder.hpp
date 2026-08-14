#pragma once

#include "SketchExtrudeGeometry.hpp"
#include "../Sketch/SketchProfileGeometryResolver.hpp"
#include "../Sketch/SketchProfileLoops.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace mir
{

enum class SketchExtrudeSurfaceKind : std::uint8_t
{
    Planar,
    Cylindrical
};

struct SketchExtrudeSideSurface
{
    std::uint32_t sourceGeometryID{0};
    SketchExtrudeSurfaceKind kind{SketchExtrudeSurfaceKind::Planar};

    SketchResolvedCurve curve{};
    SketchExtrudeVector3D direction{};
    double distance{0.0};
};

struct SketchExtrudeSurfaceSet
{
    std::vector<SketchExtrudeSideSurface> sideSurfaces;

    [[nodiscard]] bool empty() const noexcept
    {
        return sideSurfaces.empty();
    }
};

/// Builds analytical side-surface descriptors for an extrusion.
/// It does not create mesh triangles and does not mutate the document.
class SketchExtrudeSurfaceBuilder
{
public:
    [[nodiscard]] static std::optional<SketchExtrudeSurfaceSet> build(
        const SketchGeometryStore& store,
        const SketchProfileLoops& loops,
        SketchExtrudeVector3D direction,
        double distance)
    {
        if (distance <= 0.0 || direction.lengthSquared() <= 1e-24)
            return std::nullopt;

        SketchExtrudeSurfaceSet result;

        for (const auto& loop : loops.loops)
        {
            if (!loop.valid || !loop.closed)
                return std::nullopt;

            for (const auto geometryID : loop.geometryIDs)
            {
                const auto curve = SketchProfileGeometryResolver::resolve(store, geometryID);
                if (!curve || curve->construction)
                    return std::nullopt;

                SketchExtrudeSideSurface surface;
                surface.sourceGeometryID = geometryID;
                surface.kind = curve->kind == SketchCurveKind::Line
                                    ? SketchExtrudeSurfaceKind::Planar
                                    : SketchExtrudeSurfaceKind::Cylindrical;
                surface.curve = *curve;
                surface.direction = direction;
                surface.distance = distance;
                result.sideSurfaces.push_back(surface);
            }
        }

        return result.empty() ? std::nullopt : std::optional<SketchExtrudeSurfaceSet>(result);
    }
};

} // namespace mir
