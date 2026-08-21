#pragma once

#include "SketchGeometry.hpp"
#include "SketchProfile.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <unordered_map>
#include <vector>

namespace mir
{

class SketchProfileDetector
{
public:
    explicit SketchProfileDetector(double tolerance = 1e-7)
        : tolerance_(tolerance)
    {
    }

    [[nodiscard]] std::optional<SketchProfile> detect(
        std::uint32_t profileID,
        const SketchGeometryStore& store,
        const std::vector<std::uint32_t>& orderedGeometryIDs) const
    {
        if (orderedGeometryIDs.empty())
            return std::nullopt;

        std::vector<Segment> segments;
        segments.reserve(orderedGeometryIDs.size());

        for (const auto id : orderedGeometryIDs)
        {
            const auto it = std::find_if(
                store.all().begin(),
                store.all().end(),
                [id](const SketchGeometry& geometry)
                {
                    return std::visit([id](const auto& item) { return item.id == id; }, geometry);
                });

            if (it == store.all().end())
                return std::nullopt;

            const auto segment = toSegment(*it);
            if (!segment.has_value())
                return std::nullopt;

            segments.push_back(*segment);
        }

        for (std::size_t i = 1; i < segments.size(); ++i)
        {
            if (!samePoint(segments[i - 1].end, segments[i].start))
                return makeInvalid(profileID, orderedGeometryIDs);
        }

        if (!samePoint(segments.back().end, segments.front().start))
            return makeInvalid(profileID, orderedGeometryIDs);

        const bool selfIntersecting = hasSelfIntersection(segments);
        const double area = polygonArea(segments);

        SketchProfile profile;
        profile.id = profileID;
        profile.geometryIDs = orderedGeometryIDs;
        profile.closed = true;
        profile.selfIntersecting = selfIntersecting;
        profile.signedArea = area;
        profile.valid = !selfIntersecting && std::abs(area) > tolerance_;
        return profile;
    }

private:
    struct Segment
    {
        std::uint32_t id{0};
        SketchPoint2D start{};
        SketchPoint2D end{};
    };

    [[nodiscard]] std::optional<Segment> toSegment(const SketchGeometry& geometry) const
    {
        return std::visit(
            [](const auto& item) -> std::optional<Segment>
            {
                using T = std::decay_t<decltype(item)>;

                if constexpr (std::is_same_v<T, SketchLine2D>)
                {
                    if (item.construction)
                        return std::nullopt;
                    return Segment{item.id, item.start, item.end};
                }
                else if constexpr (std::is_same_v<T, SketchCircle2D>)
                {
                    // A full circle is a closed profile by itself, but is not
                    // representable as an ordered segment pair. Handle it in
                    // the dedicated profile path later.
                    return std::nullopt;
                }
                else if constexpr (std::is_same_v<T, SketchArc2D>)
                {
                    if (item.construction)
                        return std::nullopt;

                    const SketchPoint2D start{
                        item.center.x + item.radius * std::cos(item.startAngle),
                        item.center.y + item.radius * std::sin(item.startAngle)};
                    const SketchPoint2D end{
                        item.center.x + item.radius * std::cos(item.endAngle),
                        item.center.y + item.radius * std::sin(item.endAngle)};
                    return Segment{item.id, start, end};
                }
            },
            geometry);
    }

    [[nodiscard]] SketchProfile makeInvalid(
        std::uint32_t id,
        const std::vector<std::uint32_t>& geometryIDs) const
    {
        SketchProfile profile;
        profile.id = id;
        profile.geometryIDs = geometryIDs;
        profile.closed = false;
        profile.valid = false;
        return profile;
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

    [[nodiscard]] double polygonArea(const std::vector<Segment>& segments) const noexcept
    {
        double sum = 0.0;
        for (const auto& segment : segments)
            sum += segment.start.x * segment.end.y - segment.end.x * segment.start.y;
        return 0.5 * sum;
    }

    [[nodiscard]] bool hasSelfIntersection(const std::vector<Segment>& segments) const noexcept
    {
        if (segments.size() < 4)
            return false;

        for (std::size_t i = 0; i < segments.size(); ++i)
        {
            for (std::size_t j = i + 1; j < segments.size(); ++j)
            {
                if (j == i + 1 || (i == 0 && j == segments.size() - 1))
                    continue;

                if (segmentsIntersect(segments[i], segments[j]))
                    return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool segmentsIntersect(const Segment& a, const Segment& b) const noexcept
    {
        const double c1 = cross(a.start, a.end, b.start);
        const double c2 = cross(a.start, a.end, b.end);
        const double c3 = cross(b.start, b.end, a.start);
        const double c4 = cross(b.start, b.end, a.end);

        const auto opposite = [](double x, double y) {
            return (x > 0.0 && y < 0.0) || (x < 0.0 && y > 0.0);
        };

        return opposite(c1, c2) && opposite(c3, c4);
    }

    double tolerance_;
};

} // namespace mir
