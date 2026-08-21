#pragma once

#include "MirEngine/BRep/Geometry/BRepGeometryStore.hpp"
#include "MirEngine/BRep/Converters/SketchCurveToBRepCurve.hpp"
#include "MirEngine/Sketch/SketchProfileGeometryResolver.hpp"

#include <optional>

namespace mir
{

class SketchExtrudeCurveGeometryBuilder
{
public:
    [[nodiscard]] static std::optional<BRepCurveHandle> addCurve(
        const SketchResolvedCurve& curve,
        BRepGeometryStore& store) noexcept
    {
        const auto converted = SketchCurveToBRepCurve::convert(curve);
        if (!converted)
            return std::nullopt;
        return store.addCurve(*converted);
    }
};

} // namespace mir
