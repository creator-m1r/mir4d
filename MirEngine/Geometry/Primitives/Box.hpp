#pragma once

#include "../Solid/Solid3.hpp"

#include <array>
#include <cmath>
#include <vector>

namespace mir
{

/// Axis-aligned rectangular solid primitive.
///
/// The primitive is represented directly as a boundary mesh so it can be
/// consumed immediately by the current Model3/Solid3 layer without inventing
/// a second geometry representation.
class Box
{
public:
    Box() = default;

    Box(Scalar width, Scalar depth, Scalar height,
        Point3 center = Point3::origin()) noexcept
        : width_(width)
        , depth_(depth)
        , height_(height)
        , center_(center)
    {
    }

    [[nodiscard]] Scalar width() const noexcept { return width_; }
    [[nodiscard]] Scalar depth() const noexcept { return depth_; }
    [[nodiscard]] Scalar height() const noexcept { return height_; }
    [[nodiscard]] Point3 center() const noexcept { return center_; }

    [[nodiscard]] bool isValid() const noexcept
    {
        return std::isfinite(width_) &&
               std::isfinite(depth_) &&
               std::isfinite(height_) &&
               width_ > Scalar(0.0) &&
               depth_ > Scalar(0.0) &&
               height_ > Scalar(0.0) &&
               center_.isFinite();
    }

    [[nodiscard]] std::array<Point3, 8> vertices() const noexcept
    {
        const Scalar hx = width_ * Scalar(0.5);
        const Scalar hy = depth_ * Scalar(0.5);
        const Scalar hz = height_ * Scalar(0.5);

        return {
            Point3{center_.x - hx, center_.y - hy, center_.z - hz},
            Point3{center_.x + hx, center_.y - hy, center_.z - hz},
            Point3{center_.x + hx, center_.y + hy, center_.z - hz},
            Point3{center_.x - hx, center_.y + hy, center_.z - hz},
            Point3{center_.x - hx, center_.y - hy, center_.z + hz},
            Point3{center_.x + hx, center_.y - hy, center_.z + hz},
            Point3{center_.x + hx, center_.y + hy, center_.z + hz},
            Point3{center_.x - hx, center_.y + hy, center_.z + hz}
        };
    }

    [[nodiscard]] Solid3 build() const
    {
        if (!isValid())
            return {};

        const auto v = vertices();
        std::vector<Point3> points(v.begin(), v.end());

        const std::vector<Solid3::Triangle> triangles = {
            {0, 2, 1}, {0, 3, 2},
            {4, 5, 6}, {4, 6, 7},
            {0, 1, 5}, {0, 5, 4},
            {1, 2, 6}, {1, 6, 5},
            {2, 3, 7}, {2, 7, 6},
            {3, 0, 4}, {3, 4, 7}
        };

        return Solid3{std::move(points), triangles};
    }

private:
    Scalar width_{0.0};
    Scalar depth_{0.0};
    Scalar height_{0.0};
    Point3 center_{Point3::origin()};
};

} // namespace mir
