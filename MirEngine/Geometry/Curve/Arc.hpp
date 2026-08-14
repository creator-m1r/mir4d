#pragma once

#include "Circle.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace mir
{

class Arc3 final : public Curve3
{
public:
    Arc3() = default;
    Arc3(const Point3& center, const Direction3& normal, Scalar radius, Scalar startAngle, Scalar endAngle) noexcept
        : center_(center), normal_(normal), radius_(std::max(Scalar(0.0), radius)), startAngle_(startAngle), endAngle_(endAngle) { buildBasis(); }

    [[nodiscard]] Point3 pointAt(Scalar t) const override { return pointAtAngle(startAngle_ + span() * t); }
    [[nodiscard]] Vector3 tangentAt(Scalar t) const override
    {
        const Scalar angle = startAngle_ + span() * t;
        return (basisU_ * (-std::sin(angle)) + basisV_ * std::cos(angle)) * (span() * radius_);
    }
    [[nodiscard]] Scalar parameterStart() const noexcept override { return 0.0; }
    [[nodiscard]] Scalar parameterEnd() const noexcept override { return 1.0; }
    [[nodiscard]] Scalar length() const noexcept override { return std::abs(span()) * radius_; }

    [[nodiscard]] Point3 closestPoint(const Point3& point) const noexcept override
    {
        const Vector3 radial = point - center_;
        const Vector3 n = normal_.vector();
        const Vector3 projected = radial - n * radial.dot(n);
        if (projected.isZero()) return pointAt(0.0);
        const Scalar angle = nearestEquivalentAngle(std::atan2(projected.dot(basisV_), projected.dot(basisU_)));
        const Scalar s = span();
        const Scalar t = s == 0.0 ? 0.0 : std::clamp((angle - startAngle_) / s, 0.0, 1.0);
        return pointAt(t);
    }

    [[nodiscard]] bool contains(const Point3& point, Scalar tolerance = 1e-9) const noexcept override
    {
        const Vector3 radial = point - center_;
        const Vector3 n = normal_.vector();
        if (std::abs(radial.dot(n)) > tolerance) return false;
        const Vector3 projected = radial - n * radial.dot(n);
        if (std::abs(projected.length() - radius_) > tolerance) return false;
        if (radius_ <= tolerance) return true;
        const Scalar angle = nearestEquivalentAngle(std::atan2(projected.dot(basisV_), projected.dot(basisU_)));
        const Scalar t = parameterForAngle(angle);
        return t >= -tolerance && t <= 1.0 + tolerance;
    }

    [[nodiscard]] Point3 boundsMin() const noexcept override { return bounds().first; }
    [[nodiscard]] Point3 boundsMax() const noexcept override { return bounds().second; }
    [[nodiscard]] const Point3& center() const noexcept { return center_; }
    [[nodiscard]] const Direction3& normal() const noexcept { return normal_; }
    [[nodiscard]] Scalar radius() const noexcept { return radius_; }
    [[nodiscard]] Scalar startAngle() const noexcept { return startAngle_; }
    [[nodiscard]] Scalar endAngle() const noexcept { return endAngle_; }
    [[nodiscard]] Scalar span() const noexcept { return endAngle_ - startAngle_; }

private:
    static constexpr double kPi = 3.141592653589793238462643383279502884;

    void buildBasis() noexcept
    {
        const Vector3 n = normal_.vector();
        Vector3 helper = std::abs(n.x) < 0.9 ? Vector3::unitX() : Vector3::unitY();
        basisU_ = n.cross(helper).normalized();
        if (basisU_.isZero()) basisU_ = Vector3::unitZ();
        basisV_ = n.cross(basisU_).normalized();
    }

    [[nodiscard]] Point3 pointAtAngle(Scalar angle) const
    {
        return center_ + (basisU_ * std::cos(angle) + basisV_ * std::sin(angle)) * radius_;
    }

    [[nodiscard]] Scalar nearestEquivalentAngle(Scalar angle) const noexcept
    {
        const Scalar twoPi = 2.0 * kPi;
        if (span() == 0.0) return startAngle_;
        Scalar result = angle;
        const Scalar minAngle = std::min(startAngle_, endAngle_);
        const Scalar maxAngle = std::max(startAngle_, endAngle_);
        while (result < minAngle) result += twoPi;
        while (result > maxAngle) result -= twoPi;
        return result;
    }

    [[nodiscard]] Scalar parameterForAngle(Scalar angle) const noexcept
    {
        const Scalar s = span();
        return s == 0.0 ? 0.0 : (angle - startAngle_) / s;
    }

    [[nodiscard]] std::pair<Point3, Point3> bounds() const noexcept
    {
        Point3 minPoint = pointAt(0.0);
        Point3 maxPoint = minPoint;
        const auto update = [&minPoint, &maxPoint](const Point3& p) {
            minPoint.x = std::min(minPoint.x, p.x); minPoint.y = std::min(minPoint.y, p.y); minPoint.z = std::min(minPoint.z, p.z);
            maxPoint.x = std::max(maxPoint.x, p.x); maxPoint.y = std::max(maxPoint.y, p.y); maxPoint.z = std::max(maxPoint.z, p.z);
        };
        update(pointAt(1.0));
        for (const Scalar angle : {0.0, 0.5 * kPi, kPi, 1.5 * kPi}) {
            const Scalar t = parameterForAngle(nearestEquivalentAngle(angle));
            if (t >= 0.0 && t <= 1.0) update(pointAt(t));
        }
        return {minPoint, maxPoint};
    }

    Point3 center_{};
    Direction3 normal_{};
    Scalar radius_{0.0};
    Scalar startAngle_{0.0};
    Scalar endAngle_{0.0};
    Vector3 basisU_{Vector3::unitX()};
    Vector3 basisV_{Vector3::unitY()};
};

} // namespace mir
