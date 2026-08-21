#pragma once

#include "SketchGeometry.hpp"
#include "SketchProfileLoops.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>

namespace mir
{

/// Adds containment/topology information to already validated profile loops.
/// The detector uses representative points and even/odd ray casting. It is
/// deliberately transient: no SketchDocument mutation occurs here.
class SketchProfileContainment
{
public:
    struct LoopTopology
    {
        std::optional<std::size_t> parent;
        std::size_t depth{0};
        bool materialBoundary{true};
    };

    struct Result
    {
        std::vector<LoopTopology> topology;

        [[nodiscard]] bool valid() const noexcept
        {
            return !topology.empty();
        }
    };

    [[nodiscard]] std::optional<Result> analyze(
        const SketchGeometryStore& store,
        const SketchProfileLoops& profiles) const
    {
        if (!profiles.valid())
            return std::nullopt;

        Result result;
        result.topology.resize(profiles.loops.size());

        std::vector<Sample> samples;
        samples.reserve(profiles.loops.size());

        for (const auto& loop : profiles.loops)
        {
            const auto sample = representativePoint(store, loop);
            if (!sample)
                return std::nullopt;
            samples.push_back(*sample);
        }

        for (std::size_t i = 0; i < profiles.loops.size(); ++i)
        {
            double smallestContainingArea = 0.0;
            std::optional<std::size_t> parent;

            for (std::size_t j = 0; j < profiles.loops.size(); ++j)
            {
                if (i == j)
                    continue;

                const double area = std::abs(profiles.loops[j].signedArea);
                if (area <= std::abs(profiles.loops[i].signedArea))
                    continue;

                if (!contains(samples[j].boundary, samples[i].point))
                    continue;

                if (!parent || area < smallestContainingArea)
                {
                    parent = j;
                    smallestContainingArea = area;
                }
            }

            result.topology[i].parent = parent;
        }

        for (std::size_t i = 0; i < result.topology.size(); ++i)
        {
            std::size_t depth = 0;
            std::optional<std::size_t> current = result.topology[i].parent;
            std::size_t guard = 0;

            while (current && guard++ <= result.topology.size())
            {
                ++depth;
                current = result.topology[*current].parent;
            }

            if (guard > result.topology.size())
                return std::nullopt;

            result.topology[i].depth = depth;
            // Even depth = material region, odd depth = void region.
            result.topology[i].materialBoundary = (depth % 2) == 0;
        }

        return result;
    }

private:
    struct Sample
    {
        SketchPoint2D point{};
        std::vector<SketchPoint2D> boundary;
    };

    [[nodiscard]] std::optional<Sample> representativePoint(
        const SketchGeometryStore& store,
        const SketchProfileLoop& loop) const
    {
        if (loop.geometryIDs.empty())
            return std::nullopt;

        for (const auto id : loop.geometryIDs)
        {
            const auto it = std::find_if(
                store.all().begin(), store.all().end(),
                [id](const SketchGeometry& geometry) {
                    return std::visit([id](const auto& item) { return item.id == id; }, geometry);
                });

            if (it == store.all().end())
                return std::nullopt;

            const auto point = std::visit(
                [](const auto& item) -> std::optional<SketchPoint2D> {
                    using T = std::decay_t<decltype(item)>;
                    if constexpr (std::is_same_v<T, SketchLine2D>)
                        return SketchPoint2D{(item.start.x + item.end.x) * 0.5,
                                             (item.start.y + item.end.y) * 0.5};
                    else if constexpr (std::is_same_v<T, SketchCircle2D>)
                        return item.center;
                    else if constexpr (std::is_same_v<T, SketchArc2D>)
                    {
                        const double a = (item.startAngle + item.endAngle) * 0.5;
                        return SketchPoint2D{item.center.x + item.radius * std::cos(a),
                                             item.center.y + item.radius * std::sin(a)};
                    }
                },
                *it);

            if (point)
            {
                std::vector<SketchPoint2D> boundary;
                for (const auto geometryID : loop.geometryIDs)
                {
                    const auto geometryIt = std::find_if(
                        store.all().begin(), store.all().end(),
                        [geometryID](const SketchGeometry& geometry) {
                            return std::visit([geometryID](const auto& item) {
                                return item.id == geometryID;
                            }, geometry);
                        });
                    if (geometryIt == store.all().end())
                        return std::nullopt;

                    std::visit([&](const auto& item) {
                        using T = std::decay_t<decltype(item)>;
                        if constexpr (std::is_same_v<T, SketchLine2D>)
                        {
                            boundary.push_back(item.start);
                            boundary.push_back(item.end);
                        }
                        else if constexpr (std::is_same_v<T, SketchCircle2D>)
                        {
                            constexpr int count = 64;
                            for (int k = 0; k < count; ++k)
                            {
                                const double a = 2.0 * M_PI * static_cast<double>(k) / count;
                                boundary.push_back({item.center.x + item.radius * std::cos(a),
                                                    item.center.y + item.radius * std::sin(a)});
                            }
                        }
                        else if constexpr (std::is_same_v<T, SketchArc2D>)
                        {
                            const int count = 32;
                            const double sweep = item.endAngle - item.startAngle;
                            for (int k = 0; k <= count; ++k)
                            {
                                const double a = item.startAngle + sweep * static_cast<double>(k) / count;
                                boundary.push_back({item.center.x + item.radius * std::cos(a),
                                                    item.center.y + item.radius * std::sin(a)});
                            }
                        }
                    }, *geometryIt);
                }
                return Sample{*point, std::move(boundary)};
            }
        }

        return std::nullopt;
    }

    [[nodiscard]] static bool contains(
        const std::vector<SketchPoint2D>& polygon,
        SketchPoint2D point) noexcept
    {
        if (polygon.size() < 3)
            return false;

        bool inside = false;
        std::size_t j = polygon.size() - 1;

        for (std::size_t i = 0; i < polygon.size(); ++i)
        {
            const auto& a = polygon[i];
            const auto& b = polygon[j];

            const bool crosses = ((a.y > point.y) != (b.y > point.y));
            if (crosses)
            {
                const double x = (b.x - a.x) * (point.y - a.y) /
                                 ((b.y - a.y) == 0.0 ? 1e-30 : (b.y - a.y)) + a.x;
                if (point.x < x)
                    inside = !inside;
            }
            j = i;
        }
        return inside;
    }
};

} // namespace mir
