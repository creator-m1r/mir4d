#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mir
{

enum class SketchConstraintType : std::uint8_t
{
    Coincident,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Tangent,
    Concentric,
    Equal,
    Symmetric,
    Distance,
    Angle,
    Radius,
    Diameter
};

struct SketchConstraint
{
    std::uint32_t id{0};
    SketchConstraintType type{SketchConstraintType::Coincident};
    std::uint32_t firstGeometry{0};
    std::uint32_t secondGeometry{0};
    double value{0.0};
    bool driving{true};
};

class SketchConstraintStore
{
public:
    std::uint32_t add(
        SketchConstraintType type,
        std::uint32_t firstGeometry,
        std::uint32_t secondGeometry = 0,
        double value = 0.0,
        bool driving = true)
    {
        const auto id = nextId_++;
        constraints_.push_back({id, type, firstGeometry, secondGeometry, value, driving});
        return id;
    }

    [[nodiscard]] const std::vector<SketchConstraint>& all() const noexcept
    {
        return constraints_;
    }

    bool remove(std::uint32_t id) noexcept
    {
        const auto it = std::find_if(
            constraints_.begin(),
            constraints_.end(),
            [id](const SketchConstraint& constraint)
            {
                return constraint.id == id;
            });

        if (it == constraints_.end())
            return false;

        constraints_.erase(it);
        return true;
    }

    void clear() noexcept
    {
        constraints_.clear();
        nextId_ = 1;
    }

private:
    std::uint32_t nextId_{1};
    std::vector<SketchConstraint> constraints_;
};

} // namespace mir
