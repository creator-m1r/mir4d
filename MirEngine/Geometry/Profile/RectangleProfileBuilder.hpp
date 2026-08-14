#pragma once

#include "Profile3.hpp"
#include "../Curve/Line3.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace mir
{

/// Creates a rectangular planar Profile3 aligned with the supplied axes.
class RectangleProfileBuilder
{
public:
    static Profile3 create(
        const Point3& origin,
        Scalar width,
        Scalar height,
        const Direction3& axisX = Direction3::unitX(),
        const Direction3& axisY = Direction3::unitY(),
        Scalar tolerance = Scalar(1e-9))
    {
        if (!std::isfinite(width) || !std::isfinite(height) ||
            width <= Scalar(0.0) || height <= Scalar(0.0))
        {
            throw std::invalid_argument("Rectangle dimensions must be finite and positive");
        }

        const Vector3 x = axisX.asVector();
        const Vector3 y = axisY.asVector();

        if (x.isZero() || y.isZero() ||
            std::abs(x.dot(y)) > tolerance)
        {
            throw std::invalid_argument("Rectangle axes must be non-zero and orthogonal");
        }

        const Point3 p0 = origin;
        const Point3 p1 = origin + x * width;
        const Point3 p3 = origin + y * height;
        const Point3 p2 = p1 + y * height;

        std::vector<CurveLoop3::CurvePtr> curves;
        curves.reserve(4);
        curves.push_back(std::make_shared<Line3>(p0, Direction3::fromVector(p1 - p0)));
        curves.push_back(std::make_shared<Line3>(p1, Direction3::fromVector(p2 - p1)));
        curves.push_back(std::make_shared<Line3>(p2, Direction3::fromVector(p3 - p2)));
        curves.push_back(std::make_shared<Line3>(p3, Direction3::fromVector(p0 - p3)));

        return Profile3(
            CurveLoop3(std::move(curves), tolerance),
            {},
            tolerance);
    }
};

} // namespace mir
