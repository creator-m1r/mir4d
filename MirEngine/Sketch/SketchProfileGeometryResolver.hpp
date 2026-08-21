#pragma once

#include "SketchGeometry.hpp"

#include <cmath>
#include <cstdint>
#include <optional>
#include <variant>

namespace mir
{

enum class SketchCurveKind : std::uint8_t
{
    Line,
    Arc,
    Circle
};

struct SketchResolvedCurve
{
    std::uint32_t geometryID{0};
    SketchCurveKind kind{SketchCurveKind::Line};
    bool construction{false};

    SketchPoint2D start{};
    SketchPoint2D end{};
    SketchPoint2D center{};

    double radius{0.0};
    double startAngle{0.0};
    double endAngle{0.0};
};

/// Resolves a geometry ID into exact mathematical curve data.
/// No tessellation and no document mutation happen here.
class SketchProfileGeometryResolver
{
public:
    [[nodiscard]] static std::optional<SketchResolvedCurve> resolve(
        const SketchGeometryStore& store,
        std::uint32_t geometryID) noexcept
    {
        const auto it = std::find_if(
            store.all().begin(),
            store.all().end(),
            [geometryID](const SketchGeometry& geometry) {
                return std::visit(
                    [geometryID](const auto& item) { return item.id == geometryID; },
                    geometry);
            });

        if (it == store.all().end())
            return std::nullopt;

        return std::visit(
            [](const auto& item) -> std::optional<SketchResolvedCurve> {
                using T = std::decay_t<decltype(item)>;

                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    return SketchResolvedCurve{
                        item.id,
                        SketchCurveKind::Line,
                        item.construction,
                        item.start,
                        item.end,
                        {},
                        0.0,
                        0.0,
                        0.0};
                }
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                {
                    return SketchResolvedCurve{
                        item.id,
                        SketchCurveKind::Arc,
                        item.construction,
                        {
                            item.center.x + item.radius * std::cos(item.startAngle),
                            item.center.y + item.radius * std::sin(item.startAngle)},
                        {
                            item.center.x + item.radius * std::cos(item.endAngle),
                            item.center.y + item.radius * std::sin(item.endAngle)},
                        item.center,
                        item.radius,
                        item.startAngle,
                        item.endAngle};
                }
                else if constexpr (std::is_same_v<T, SketchCircle2D>)
                {
                    return SketchResolvedCurve{
                        item.id,
                        SketchCurveKind::Circle,
                        item.construction,
                        {},
                        {},
                        item.center,
                        item.radius,
                        0.0,
                        2.0 * std::acos(-1.0)};
                }
            },
            *it);
    }
};

} // namespace mir
