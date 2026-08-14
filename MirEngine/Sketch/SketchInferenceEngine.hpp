#pragma once

#include "SketchConstraint.hpp"
#include "SketchGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace mir
{

enum class SketchInferenceType : std::uint8_t
{
    None,
    Horizontal,
    Vertical,
    Coincident,
    Midpoint,
    Perpendicular
};

struct SketchInference
{
    SketchInferenceType type{SketchInferenceType::None};
    std::uint32_t firstGeometryId{0};
    std::uint32_t secondGeometryId{0};
    double confidence{0.0};
};

class SketchInferenceEngine
{
public:
    explicit SketchInferenceEngine(double angularTolerance = 0.08,
                                   double pointTolerance = 8.0)
        : angularTolerance_(std::max(0.0, angularTolerance)),
          pointTolerance_(std::max(0.0, pointTolerance))
    {
    }

    [[nodiscard]] std::vector<SketchInference> inferLine(
        const SketchGeometryStore& geometry,
        std::uint32_t lineId,
        SketchPoint2D start,
        SketchPoint2D end) const noexcept
    {
        std::vector<SketchInference> result;

        const double dx = end.x - start.x;
        const double dy = end.y - start.y;
        const double length = std::sqrt(dx * dx + dy * dy);

        if (length <= 1e-9)
            return result;

        const double horizontalError = std::abs(dy) / length;
        const double verticalError = std::abs(dx) / length;

        if (horizontalError <= angularTolerance_)
        {
            result.push_back({
                SketchInferenceType::Horizontal,
                lineId,
                0,
                1.0 - horizontalError / angularTolerance_});
        }

        if (verticalError <= angularTolerance_)
        {
            result.push_back({
                SketchInferenceType::Vertical,
                lineId,
                0,
                1.0 - verticalError / angularTolerance_});
        }

        for (const auto& item : geometry.all())
        {
            std::visit([&](const auto& value)
            {
                if (value.id == lineId)
                    return;

                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    const auto startDistance = distance(start, value.start);
                    const auto endDistance = distance(end, value.end);

                    if (startDistance <= pointTolerance_)
                    {
                        result.push_back({
                            SketchInferenceType::Coincident,
                            lineId,
                            value.id,
                            confidence(pointTolerance_, startDistance)});
                    }

                    if (endDistance <= pointTolerance_)
                    {
                        result.push_back({
                            SketchInferenceType::Coincident,
                            lineId,
                            value.id,
                            confidence(pointTolerance_, endDistance)});
                    }
                }
            }, item);
        }

        std::sort(result.begin(), result.end(),
            [](const SketchInference& a, const SketchInference& b)
            {
                return a.confidence > b.confidence;
            });

        return result;
    }

private:
    [[nodiscard]] static double distance(SketchPoint2D a, SketchPoint2D b) noexcept
    {
        const double dx = a.x - b.x;
        const double dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    [[nodiscard]] static double confidence(double tolerance, double value) noexcept
    {
        if (tolerance <= 0.0)
            return 0.0;
        return std::clamp(1.0 - value / tolerance, 0.0, 1.0);
    }

    double angularTolerance_;
    double pointTolerance_;
};

} // namespace mir
