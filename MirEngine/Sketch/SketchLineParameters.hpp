#pragma once

#include "SketchGeometry.hpp"

#include <cmath>
#include <cstdint>
#include <optional>

namespace mir
{

struct SketchLineParameters
{
    std::uint32_t geometryId{0};
    double startX{0.0};
    double startY{0.0};
    double endX{0.0};
    double endY{0.0};
    double length{0.0};
    double angleRadians{0.0};
    bool horizontal{false};
    bool vertical{false};
};

class SketchLineParameterReader
{
public:
    [[nodiscard]] static std::optional<SketchLineParameters> read(
        const SketchGeometryStore& geometry,
        std::uint32_t geometryId) noexcept
    {
        const auto item = geometry.find(geometryId);
        if (!item)
            return std::nullopt;

        const auto* line = std::get_if<SketchLine2D>(&*item);
        if (!line)
            return std::nullopt;

        const double dx = line->end.x - line->start.x;
        const double dy = line->end.y - line->start.y;
        const double length = std::sqrt(dx * dx + dy * dy);
        const double angle = std::atan2(dy, dx);

        return SketchLineParameters{
            line->id,
            line->start.x,
            line->start.y,
            line->end.x,
            line->end.y,
            length,
            angle,
            std::abs(dy) <= 1e-9,
            std::abs(dx) <= 1e-9};
    }
};

} // namespace mir
