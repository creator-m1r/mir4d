// MirEngine/Math/Transform.hpp
// Позиция, вращение и масштаб объекта в одном canonical типе.
// C++23

#pragma once

#include "../Core/Types/Scalar.hpp"
#include "Point.hpp"
#include "Vector/Vector.hpp"
#include "Quaternion/Quaternion.hpp"
#include "TransformMatrix.hpp"

#include <cmath>

namespace mir4d
{

/// Canonical engineering transform.
///
/// Lower-level mathematical value types are still temporarily provided by
/// the legacy mir namespace. Transform ownership itself belongs to mir4d.
class Transform
{
public:
    mir::Point3 position{};
    mir::Quaternion rotation = mir::Quaternion::identity();
    mir::Vector3 scale{1.0, 1.0, 1.0};

    constexpr Transform() noexcept = default;
    constexpr Transform(const mir::Point3& pos,
                        const mir::Quaternion& rot = mir::Quaternion::identity(),
                        const mir::Vector3& scl = mir::Vector3{1.0, 1.0, 1.0}) noexcept
        : position(pos), rotation(rot), scale(scl) {}

    [[nodiscard]] static constexpr Transform identity() noexcept { return {}; }

    [[nodiscard]] bool isValid() const noexcept
    {
        return position.isFinite() && rotation.isFinite() && scale.isFinite();
    }

    [[nodiscard]] mir::Matrix4 matrix() const noexcept
    {
        const mir::Matrix4 scaleMatrix = mir::Matrix4::scale(scale);
        const mir::Matrix4 rotationMatrix = rotation.toMatrix4();
        const mir::Matrix4 translationMatrix =
            mir::Matrix4::translation(position.x, position.y, position.z);
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    [[nodiscard]] mir::Point3 transformPoint(const mir::Point3& localPoint) const noexcept
    {
        const mir::Vector3 localVector{localPoint.x, localPoint.y, localPoint.z};
        const mir::Vector3 worldVector = matrix().transformPoint(localVector);
        return {worldVector.x, worldVector.y, worldVector.z};
    }

    [[nodiscard]] mir::Vector3 transformDirection(const mir::Vector3& localDirection) const noexcept
    {
        return matrix().transformDirection(localDirection);
    }

    [[nodiscard]] Transform combine(const Transform& other) const noexcept
    {
        Transform result;
        result.scale = mir::Vector3::componentMul(scale, other.scale);
        result.rotation = rotation * other.rotation;
        result.position = transformPoint(other.position);
        return result;
    }

    [[nodiscard]] Transform inverse() const noexcept
    {
        Transform result;
        result.scale = {
            safeReciprocal(scale.x),
            safeReciprocal(scale.y),
            safeReciprocal(scale.z)};

        result.rotation = rotation.inverse();
        const mir::Vector3 negativePosition{
            -position.x,
            -position.y,
            -position.z};
        mir::Vector3 transformed = result.rotation.rotate(negativePosition);
        transformed.x *= result.scale.x;
        transformed.y *= result.scale.y;
        transformed.z *= result.scale.z;
        result.position = {transformed.x, transformed.y, transformed.z};
        return result;
    }

    friend constexpr bool operator==(const Transform& a, const Transform& b) noexcept
    {
        return a.position == b.position &&
               a.rotation == b.rotation &&
               a.scale == b.scale;
    }

    friend constexpr bool operator!=(const Transform& a, const Transform& b) noexcept
    {
        return !(a == b);
    }

private:
    [[nodiscard]] static mir::Scalar safeReciprocal(mir::Scalar value) noexcept
    {
        constexpr mir::Scalar epsilon = mir::Scalar(1e-20);
        if (std::abs(value) <= epsilon)
            return mir::Scalar(0.0);
        return mir::Scalar(1.0) / value;
    }
};

} // namespace mir4d

namespace mir
{

/// Compatibility alias. Transform is owned by mir4d.
using Transform = mir4d::Transform;

} // namespace mir
