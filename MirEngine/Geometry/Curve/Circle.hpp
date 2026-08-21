#pragma once

#include "Curve.hpp"
#include "../Direction/Direction.hpp"
#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

class Circle3 final : public Curve3
{
public:
    Circle3() = default;
    Circle3(const Point3& center, const Direction3& normal, Scalar radius) noexcept
        : center_(center), normal_(normal), radius_(std::max(Scalar(0.0), radius)) { buildBasis(); }

    [[nodiscard]] Point3 pointAt(Scalar t) const override { return pointAtAngle(Scalar(2.0 * kPi) * t); }
    [[nodiscard]] Vector3 tangentAt(Scalar t) const override
    {
        const Scalar angle = Scalar(2.0 * kPi) * t;
        return (basisU_ * (-std::sin(angle)) + basisV_ * std::cos(angle)) * Scalar(2.0 * kPi * radius_);
    }
    [[nodiscard]] Scalar parameterStart() const noexcept override { return 0.0; }
    [[nodiscard]] Scalar parameterEnd() const noexcept override { return 1.0; }
    [[nodiscard]] Scalar length() const noexcept override { return Scalar(2.0 * kPi) * radius_; }

    [[nodiscard]] Point3 closestPoint(const Point3& point) const noexcept override
    {
        const Vector3 radial = point - center_;
        const Vector3 n = normal_.asVector();
        const Vector3 projected = radial - n * radial.dot(n);
        if (projected.isZero()) return center_ + basisU_ * radius_;
        return center_ + projected.normalized() * radius_;
    }

    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-9) const noexcept override
    {
        const Vector3 radial = point - center_;
        const Vector3 n = normal_.asVector();
        const Scalar planeDistance = std::abs(radial.dot(n));
        const Scalar radialSq = std::max(Scalar(0.0), radial.lengthSquared() - planeDistance * planeDistance);
        return planeDistance <= tolerance && std::abs(std::sqrt(radialSq) - radius_) <= tolerance;
    }

    [[nodiscard]] Point3 boundsMin() const noexcept override
    {
        const Vector3 n = normal_.asVector();
        return center_ - Vector3{
            radius_ * std::sqrt(std::max(Scalar(0.0), Scalar(1.0) - n.x * n.x)),
            radius_ * std::sqrt(std::max(Scalar(0.0), Scalar(1.0) - n.y * n.y)),
            radius_ * std::sqrt(std::max(Scalar(0.0), Scalar(1.0) - n.z * n.z))};
    }

    [[nodiscard]] Point3 boundsMax() const noexcept override
    {
        const Vector3 n = normal_.asVector();
        return center_ + Vector3{
            radius_ * std::sqrt(std::max(Scalar(0.0), Scalar(1.0) - n.x * n.x)),
            radius_ * std::sqrt(std::max(Scalar(0.0), Scalar(1.0) - n.y * n.y)),
            radius_ * std::sqrt(std::max(Scalar(0.0), Scalar(1.0) - n.z * n.z))};
    }

    [[nodiscard]] const Point3& center() const noexcept { return center_; }
    [[nodiscard]] const Direction3& normal() const noexcept { return normal_; }
    [[nodiscard]] Scalar radius() const noexcept { return radius_; }

protected:
    static constexpr double kPi = 3.141592653589793238462643383279502884;

    void buildBasis() noexcept
    {
        const Vector3 n = normal_.asVector();
        Vector3 helper = std::abs(n.x) < Scalar(0.9) ? Vector3::unitX() : Vector3::unitY();
        basisU_ = n.cross(helper).normalized();
        if (basisU_.isZero()) basisU_ = Vector3::unitZ();
        basisV_ = n.cross(basisU_).normalized();
    }

    [[nodiscard]] Point3 pointAtAngle(Scalar angle) const
    {
        return center_ + (basisU_ * std::cos(angle) + basisV_ * std::sin(angle)) * radius_;
    }

    Point3 center_{};
    Direction3 normal_{};
    Scalar radius_{0.0};
    Vector3 basisU_{Vector3::unitX()};
    Vector3 basisV_{Vector3::unitY()};
};

}
