#pragma once

#include "SketchGeometry.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <vector>

namespace mir
{

enum class SketchSnapType : std::uint8_t
{
    None,
    Endpoint,
    Midpoint,
    Center,
    Intersection,
    Horizontal,
    Vertical,
    Perpendicular,
    Tangent,
    Coincident
};

struct SketchSnapCandidate
{
    SketchSnapType type{SketchSnapType::None};
    std::uint32_t geometryId{0};
    SketchPoint2D point{};
    double distance{std::numeric_limits<double>::max()};
};

class SketchSnapEngine
{
public:
    explicit SketchSnapEngine(double tolerance = 8.0)
        : tolerance_(std::max(0.0, tolerance))
    {
    }

    [[nodiscard]] std::optional<SketchSnapCandidate> nearest(
        const SketchGeometryStore& geometry,
        SketchPoint2D cursor) const noexcept
    {
        std::optional<SketchSnapCandidate> best;

        for (const auto& item : geometry.all())
        {
            std::visit([&](const auto& value)
            {
                using T = std::decay_t<decltype(value)>;

                auto consider = [&](SketchSnapType type, SketchPoint2D point)
                {
                    const double dx = point.x - cursor.x;
                    const double dy = point.y - cursor.y;
                    const double distance = std::sqrt(dx * dx + dy * dy);

                    if (distance > tolerance_)
                        return;

                    if (!best || distance < best->distance)
                    {
                        best = SketchSnapCandidate{type, value.id, point, distance};
                    }
                };

                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    consider(SketchSnapType::Endpoint, value.start);
                    consider(SketchSnapType::Endpoint, value.end);
                    consider(SketchSnapType::Midpoint,
                        SketchPoint2D{
                            (value.start.x + value.end.x) * 0.5,
                            (value.start.y + value.end.y) * 0.5});
                }
                else if constexpr (std::is_same_v<T, SketchCircle2D> ||
                                   std::is_same_v<T, SketchArc2D>)
                {
                    consider(SketchSnapType::Center, value.center);
                }
            }, item);
        }

        return best;
    }

private:
    double tolerance_;
};

} // namespace mir
