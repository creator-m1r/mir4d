#pragma once

#include "SketchGeometry.hpp"
#include "SketchProfileLoops.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace mir
{

class SketchProfileLoopDetector
{
public:
    explicit SketchProfileLoopDetector(double tolerance = 1e-7)
        : tolerance_(tolerance)
    {
    }

    [[nodiscard]] std::optional<SketchProfileLoops> detect(
        const SketchGeometryStore& store,
        const std::vector<std::vector<std::uint32_t>>& candidateLoops) const
    {
        if (candidateLoops.empty())
            return std::nullopt;

        SketchProfileLoops result;
        result.loops.reserve(candidateLoops.size());

        for (const auto& ids : candidateLoops)
        {
            const auto loop = detectLoop(store, ids);
            if (!loop)
                return std::nullopt;
            result.loops.push_back(*loop);
        }

        double largest = -1.0;
        for (std::size_t i = 0; i < result.loops.size(); ++i)
        {
            if (std::abs(result.loops[i].signedArea) > largest)
            {
                largest = std::abs(result.loops[i].signedArea);
                result.outerLoopIndex = i;
            }
        }

        return result.valid() ? std::optional<SketchProfileLoops>(result) : std::nullopt;
    }

private:
    struct Segment
    {
        SketchPoint2D start{};
        SketchPoint2D end{};
        std::uint32_t id{0};
    };

    [[nodiscard]] std::optional<SketchProfileLoop> detectLoop(
        const SketchGeometryStore& store,
        const std::vector<std::uint32_t>& ids) const
    {
        if (ids.empty())
            return std::nullopt;

        if (ids.size() == 1)
        {
            const auto* geometry = find(store, ids.front());
            if (!geometry)
                return std::nullopt;

            if (const auto* circle = std::get_if<SketchCircle2D>(geometry))
            {
                if (circle->construction || circle->radius <= tolerance_)
                    return std::nullopt;

                return SketchProfileLoop{
                    {circle->id},
                    M_PI * circle->radius * circle->radius,
                    true,
                    true};
            }
        }

        std::vector<Segment> segments;
        segments.reserve(ids.size());

        for (const auto id : ids)
        {
            const auto* geometry = find(store, id);
            if (!geometry)
                return std::nullopt;

            const auto segment = endpoints(*geometry);
            if (!segment)
                return std::nullopt;
            segments.push_back(*segment);
        }

        for (std::size_t i = 1; i < segments.size(); ++i)
        {
            if (!samePoint(segments[i - 1].end, segments[i].start))
                return invalid(ids);
        }

        if (!samePoint(segments.back().end, segments.front().start))
            return invalid(ids);

        const auto area = areaOf(segments);
        if (std::abs(area) <= tolerance_)
            return invalid(ids);

        return SketchProfileLoop{ids, area, true, !selfIntersecting(segments)};
    }

    [[nodiscard]] const SketchGeometry* find(
        const SketchGeometryStore& store,
        std::uint32_t id) const noexcept
    {
        const auto it = std::find_if(
            store.all().begin(),
            store.all().end(),
            [id](const SketchGeometry& geometry) {
                return std::visit([id](const auto& item) { return item.id == id; }, geometry);
            });
        return it == store.all().end() ? nullptr : &*it;
    }

    [[nodiscard]] std::optional<Segment> endpoints(const SketchGeometry& geometry) const
    {
        return std::visit(
            [&](const auto& item) -> std::optional<Segment> {
                using T = std::decay_t<decltype(item)>;

                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    if (item.construction)
                        return std::nullopt;
                    return Segment{item.start, item.end, item.id};
                }
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                {
                    if (item.construction || item.radius <= tolerance_)
                        return std::nullopt;

                    const SketchPoint2D start{
                        item.center.x + item.radius * std::cos(item.startAngle),
                        item.center.y + item.radius * std::sin(item.startAngle)};
                    const SketchPoint2D end{
                        item.center.x + item.radius * std::cos(item.endAngle),
                        item.center.y + item.radius * std::sin(item.endAngle)};
                    return Segment{start, end, item.id};
                }
                else
                {
                    return std::nullopt;
                }
            },
            geometry);
    }

    [[nodiscard]] SketchProfileLoop invalid(const std::vector<std::uint32_t>& ids) const
    {
        return SketchProfileLoop{ids, 0.0, false, false};
    }

    [[nodiscard]] bool samePoint(SketchPoint2D a, SketchPoint2D b) const noexcept
    {
        return std::abs(a.x - b.x) <= tolerance_ &&
               std::abs(a.y - b.y) <= tolerance_;
    }

    [[nodiscard]] static double cross(SketchPoint2D a, SketchPoint2D b, SketchPoint2D c) noexcept
    {
        return (b.x - a.x) * (c.y - a.y) -
               (b.y - a.y) * (c.x - a.x);
    }

    [[nodiscard]] static double areaOf(const std::vector<Segment>& segments) noexcept
    {
        double sum = 0.0;
        for (const auto& segment : segments)
            sum += segment.start.x * segment.end.y - segment.end.x * segment.start.y;
        return 0.5 * sum;
    }

    [[nodiscard]] bool selfIntersecting(const std::vector<Segment>& segments) const noexcept
    {
        if (segments.size() < 4)
            return false;

        for (std::size_t i = 0; i < segments.size(); ++i)
        {
            for (std::size_t j = i + 1; j < segments.size(); ++j)
            {
                if (j == i + 1 || (i == 0 && j == segments.size() - 1))
                    continue;

                const auto& a = segments[i];
                const auto& b = segments[j];
                const double c1 = cross(a.start, a.end, b.start);
                const double c2 = cross(a.start, a.end, b.end);
                const double c3 = cross(b.start, b.end, a.start);
                const double c4 = cross(b.start, b.end, a.end);

                if (((c1 > tolerance_ && c2 < -tolerance_) ||
                     (c1 < -tolerance_ && c2 > tolerance_)) &&
                    ((c3 > tolerance_ && c4 < -tolerance_) ||
                     (c3 < -tolerance_ && c4 > tolerance_)))
                    return true;
            }
        }
        return false;
    }

    double tolerance_;
};

}
