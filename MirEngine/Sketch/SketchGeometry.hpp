#pragma once

#include <algorithm>
#include <cstdint>
#include <variant>
#include <vector>

namespace mir
{

struct SketchPoint2D
{
    double x{0.0};
    double y{0.0};
};

struct SketchLine2D
{
    std::uint32_t id{0};
    SketchPoint2D start{};
    SketchPoint2D end{};
    bool construction{false};
};

struct SketchArc2D
{
    std::uint32_t id{0};
    SketchPoint2D center{};
    double radius{1.0};
    double startAngle{0.0};
    double endAngle{0.0};
    bool construction{false};
};

struct SketchCircle2D
{
    std::uint32_t id{0};
    SketchPoint2D center{};
    double radius{1.0};
    bool construction{false};
};

/// True spline primitive. Control points define a natural cubic spline built
/// through them (see mir::math::buildCubicSpline). The curve is a decorative /
/// driven geometry: the constraint solver preserves it as-is and does not move
/// its control points, so it never introduces under-constrained variables.
struct SketchSpline2D
{
    std::uint32_t id{0};
    std::vector<SketchPoint2D> controlPoints{};
    bool closed{false};
    bool construction{false};
};

using SketchGeometry = std::variant<SketchLine2D, SketchArc2D, SketchCircle2D, SketchSpline2D>;

class SketchGeometryStore
{
public:
    std::uint32_t addLine(SketchPoint2D start, SketchPoint2D end, bool construction = false)
    {
        const auto id = nextId_++;
        geometries_.emplace_back(SketchLine2D{id, start, end, construction});
        return id;
    }

    std::uint32_t addCircle(SketchPoint2D center, double radius, bool construction = false)
    {
        const auto id = nextId_++;
        geometries_.emplace_back(SketchCircle2D{id, center, std::max(0.0, radius), construction});
        return id;
    }

    std::uint32_t addArc(
        SketchPoint2D center,
        double radius,
        double startAngle,
        double endAngle,
        bool construction = false)
    {
        const auto id = nextId_++;
        geometries_.emplace_back(SketchArc2D{
            id,
            center,
            std::max(0.0, radius),
            startAngle,
            endAngle,
            construction});
        return id;
    }

    std::uint32_t addSpline(
        std::vector<SketchPoint2D> controlPoints,
        bool closed = false,
        bool construction = false)
    {
        const auto id = nextId_++;
        SketchSpline2D spline;
        spline.id = id;
        spline.controlPoints = std::move(controlPoints);
        spline.closed = closed;
        spline.construction = construction;
        geometries_.emplace_back(std::move(spline));
        return id;
    }

    [[nodiscard]] const std::vector<SketchGeometry>& all() const noexcept
    {
        return geometries_;
    }

    [[nodiscard]] std::vector<SketchGeometry>& mutableAllForSolver() noexcept
    {
        return geometries_;
    }

    /// Inserts an already built geometry variant. A zero id is replaced by a
    /// fresh store id so command payloads can carry unassigned entities.
    std::uint32_t add(SketchGeometry geometry)
    {
        const auto existingId = std::visit(
            [](const auto& item) { return item.id; },
            geometry);

        if (existingId == 0)
        {
            const auto id = nextId_++;
            std::visit(
                [id](auto& item) { item.id = id; },
                geometry);
            geometries_.push_back(std::move(geometry));
            return id;
        }

        geometries_.push_back(std::move(geometry));
        return existingId;
    }

    [[nodiscard]] const SketchGeometry* find(std::uint32_t id) const noexcept
    {
        const auto it = std::find_if(
            geometries_.begin(),
            geometries_.end(),
            [id](const SketchGeometry& geometry)
            {
                return std::visit(
                    [id](const auto& value) { return value.id == id; },
                    geometry);
            });

        if (it == geometries_.end())
            return nullptr;

        return &*it;
    }

    [[nodiscard]] SketchGeometry* findMutable(std::uint32_t id) noexcept
    {
        const auto it = std::find_if(
            geometries_.begin(),
            geometries_.end(),
            [id](const SketchGeometry& geometry)
            {
                return std::visit(
                    [id](const auto& value) { return value.id == id; },
                    geometry);
            });

        if (it == geometries_.end())
            return nullptr;

        return &*it;
    }

    bool remove(std::uint32_t id) noexcept
    {
        const auto it = std::find_if(
            geometries_.begin(),
            geometries_.end(),
            [id](const SketchGeometry& geometry)
            {
                return std::visit(
                    [id](const auto& value) { return value.id == id; },
                    geometry);
            });

        if (it == geometries_.end())
            return false;

        geometries_.erase(it);
        return true;
    }

    void clear() noexcept
    {
        geometries_.clear();
        nextId_ = 1;
    }

private:
    std::uint32_t nextId_{1};
    std::vector<SketchGeometry> geometries_;
};

} // namespace mir
