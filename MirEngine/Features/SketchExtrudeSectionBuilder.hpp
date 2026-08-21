#pragma once

#include "SketchExtrudeFeature.hpp"
#include "SketchExtrudeGeometry.hpp"
#include "../Sketch/SketchProfileGeometryResolver.hpp"
#include "../Sketch/SketchProfileLoops.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace mir
{

struct SketchExtrudeCurveSection
{
    std::uint32_t geometryID{0};
    SketchCurveKind kind{SketchCurveKind::Line};
    SketchExtrudePoint3D start{};
    SketchExtrudePoint3D end{};
    SketchExtrudePoint3D center{};
    double radius{0.0};
    double startAngle{0.0};
    double endAngle{0.0};
};

struct SketchExtrudeSection
{
    double offset{0.0};
    std::vector<SketchExtrudeCurveSection> curves;
};

struct SketchExtrudeSections
{
    SketchExtrudeSection bottom;
    SketchExtrudeSection top;
};

class SketchExtrudeSectionBuilder
{
public:
    [[nodiscard]] static std::optional<SketchExtrudeSections> build(
        const SketchExtrudeFeature& feature,
        const SketchProfileLoops& loops,
        const SketchGeometryStore& store) noexcept
    {
        if (!feature.valid() || !loops.valid())
            return std::nullopt;

        SketchExtrudeSections sections;
        sections.bottom.offset = feature.startOffset();
        sections.top.offset = feature.endOffset();

        if (!resolveRegions(loops, store, sections.bottom) ||
            !resolveRegions(loops, store, sections.top))
            return std::nullopt;

        return sections;
    }

private:
    [[nodiscard]] static SketchExtrudePoint3D lift(
        SketchPoint2D point,
        double offset) noexcept
    {
        return {point.x, point.y, offset};
    }

    [[nodiscard]] static bool appendCurve(
        const SketchResolvedCurve& curve,
        double offset,
        SketchExtrudeSection& section) noexcept
    {
        SketchExtrudeCurveSection result;
        result.geometryID = curve.geometryID;
        result.kind = curve.kind;
        result.radius = curve.radius;
        result.startAngle = curve.startAngle;
        result.endAngle = curve.endAngle;
        result.center = lift(curve.center, offset);

        if (curve.kind == SketchCurveKind::Circle)
        {
            result.start = result.center;
            result.end = result.center;
        }
        else
        {
            result.start = lift(curve.start, offset);
            result.end = lift(curve.end, offset);
        }

        section.curves.push_back(result);
        return true;
    }

    [[nodiscard]] static bool resolveLoop(
        const SketchProfileLoop& loop,
        const SketchGeometryStore& store,
        double offset,
        SketchExtrudeSection& section) noexcept
    {
        if (!loop.closed || !loop.valid)
            return false;

        for (const auto id : loop.geometryIDs)
        {
            const auto curve = SketchProfileGeometryResolver::resolve(store, id);
            if (!curve || curve->construction)
                return false;

            if (!appendCurve(*curve, offset, section))
                return false;
        }
        return true;
    }

    [[nodiscard]] static bool resolveRegions(
        const SketchProfileLoops& loops,
        const SketchGeometryStore& store,
        SketchExtrudeSection& section) noexcept
    {
        for (const auto& loop : loops.loops)
        {
            if (!resolveLoop(loop, store, section.offset, section))
                return false;
        }
        return !section.curves.empty();
    }
};

}
