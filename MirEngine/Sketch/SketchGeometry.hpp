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

using SketchGeometry = std::variant<SketchLine2D, SketchArc2D, SketchCircle2D>;

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

    [[nodiscard]] const std::vector<SketchGeometry>& all() const noexcept
    {
        return geometries_;
    }

    [[nodiscard]] std::vector<SketchGeometry>& mutableAllForSolver() noexcept
    {
        return geometries_;
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
