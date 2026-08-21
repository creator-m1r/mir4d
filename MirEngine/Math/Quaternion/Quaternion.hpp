// MirEngine/Math/Quaternion/Quaternion.hpp
// Quaternion for stable 3D rotations.
// C++23, no external dependencies.

#pragma once

#include "../../Core/Types/Angle.hpp"
#include "../../Core/Types/Scalar.hpp"
#include "../SpatialMatrix.hpp"
#include "../TransformMatrix.hpp"
#include "../Vector/Vector.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

class Quaternion
{
public:
    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};
    Scalar w{1.0};

    constexpr Quaternion() noexcept = default;

    constexpr Quaternion(Scalar x_, Scalar y_, Scalar z_, Scalar w_) noexcept
        : x(x_), y(y_), z(z_), w(w_) {}

    [[nodiscard]] static constexpr Quaternion identity() noexcept
    {
        return {0.0, 0.0, 0.0, 1.0};
    }

    [[nodiscard]] static Quaternion fromAxisAngle(
        const Vector3& axis,
        const Angle& angle,
        Scalar epsilon = Scalar(1e-20)) noexcept
    {
        const Scalar axisLength = axis.length();
        if (!std::isfinite(axisLength) || axisLength <= epsilon)
            return identity();

        const Vector3 unitAxis = axis / axisLength;
        const Scalar halfAngle = angle.radians() * Scalar(0.5);
        const Scalar s = std::sin(halfAngle);
        const Scalar c = std::cos(halfAngle);

        return {unitAxis.x * s, unitAxis.y * s, unitAxis.z * s, c};
    }

    [[nodiscard]] static Quaternion fromEuler(
        const Angle& yaw,
        const Angle& pitch,
        const Angle& roll) noexcept
    {
        const Scalar halfYaw = yaw.radians() * Scalar(0.5);
        const Scalar halfPitch = pitch.radians() * Scalar(0.5);
        const Scalar halfRoll = roll.radians() * Scalar(0.5);

        const Scalar cy = std::cos(halfYaw);
        const Scalar sy = std::sin(halfYaw);
        const Scalar cp = std::cos(halfPitch);
        const Scalar sp = std::sin(halfPitch);
        const Scalar cr = std::cos(halfRoll);
        const Scalar sr = std::sin(halfRoll);

        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy};
    }

    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept
    {
        return x * x + y * y + z * z + w * w;
    }

    [[nodiscard]] Scalar length() const noexcept
    {
        return std::sqrt(lengthSquared());
    }

    [[nodiscard]] Quaternion normalized(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar len = length();
        if (!std::isfinite(len) || len <= epsilon)
            return identity();

        const Scalar invLength = Scalar(1.0) / len;
        return {x * invLength, y * invLength, z * invLength, w * invLength};
    }

    bool normalize(Scalar epsilon = Scalar(1e-20)) noexcept
    {
        const Scalar len = length();
        if (!std::isfinite(len) || len <= epsilon)
        {
            *this = identity();
            return false;
        }

        const Scalar invLength = Scalar(1.0) / len;
        x *= invLength;
        y *= invLength;
        z *= invLength;
        w *= invLength;
        return true;
    }

    [[nodiscard]] Quaternion conjugated() const noexcept
    {
        return {-x, -y, -z, w};
    }

    [[nodiscard]] Quaternion inverse(Scalar epsilon = Scalar(1e-20)) const noexcept
    {
        const Scalar normSquared = lengthSquared();
        if (!std::isfinite(normSquared) || normSquared <= epsilon)
            return identity();

        const Scalar invNormSquared = Scalar(1.0) / normSquared;
        return {-x * invNormSquared, -y * invNormSquared, -z * invNormSquared, w * invNormSquared};
    }

    friend constexpr Quaternion operator*(const Quaternion& a, const Quaternion& b) noexcept
    {
        return {
            a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
            a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
            a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
            a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
    }

    friend constexpr Quaternion operator-(const Quaternion& q) noexcept
    {
        return {-q.x, -q.y, -q.z, -q.w};
    }

    [[nodiscard]] Vector3 rotate(const Vector3& vector) const noexcept
    {
        const Quaternion q = normalized();
        const Vector3 qVector{q.x, q.y, q.z};
        const Vector3 uv = Vector3::cross(qVector, vector);
        const Vector3 uuv = Vector3::cross(qVector, uv);
        return vector + (uv * q.w + uuv) * Scalar(2.0);
    }

    [[nodiscard]] Matrix3 toMatrix3() const noexcept
    {
        const Quaternion q = normalized();
        const Scalar xx = q.x * q.x;
        const Scalar yy = q.y * q.y;
        const Scalar zz = q.z * q.z;
        const Scalar xy = q.x * q.y;
        const Scalar xz = q.x * q.z;
        const Scalar yz = q.y * q.z;
        const Scalar wx = q.w * q.x;
        const Scalar wy = q.w * q.y;
        const Scalar wz = q.w * q.z;

        return Matrix3{
            Scalar(1.0) - Scalar(2.0) * (yy + zz), Scalar(2.0) * (xy - wz), Scalar(2.0) * (xz + wy),
            Scalar(2.0) * (xy + wz), Scalar(1.0) - Scalar(2.0) * (xx + zz), Scalar(2.0) * (yz - wx),
            Scalar(2.0) * (xz - wy), Scalar(2.0) * (yz + wx), Scalar(1.0) - Scalar(2.0) * (xx + yy)};
    }

    [[nodiscard]] Matrix4 toMatrix4() const noexcept
    {
        const Matrix3 rotation = toMatrix3();
        return Matrix4{
            rotation(0, 0), rotation(0, 1), rotation(0, 2), Scalar(0.0),
            rotation(1, 0), rotation(1, 1), rotation(1, 2), Scalar(0.0),
            rotation(2, 0), rotation(2, 1), rotation(2, 2), Scalar(0.0),
            Scalar(0.0), Scalar(0.0), Scalar(0.0), Scalar(1.0)};
    }

    [[nodiscard]] static Quaternion slerp(const Quaternion& first, const Quaternion& second, Scalar t) noexcept
    {
        const Quaternion q1 = first.normalized();
        Quaternion q2 = second.normalized();
        Scalar dot = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
        dot = std::clamp(dot, Scalar(-1.0), Scalar(1.0));

        if (dot < Scalar(0.0))
        {
            q2 = -q2;
            dot = -dot;
        }

        const Scalar clampedT = std::clamp(t, Scalar(0.0), Scalar(1.0));
        if (dot > Scalar(0.9995))
        {
            Quaternion result{
                q1.x + clampedT * (q2.x - q1.x),
                q1.y + clampedT * (q2.y - q1.y),
                q1.z + clampedT * (q2.z - q1.z),
                q1.w + clampedT * (q2.w - q1.w)};
            return result.normalized();
        }

        const Scalar theta0 = std::acos(dot);
        const Scalar sinTheta0 = std::sin(theta0);
        if (std::abs(sinTheta0) <= Scalar(1e-20))
            return q1;

        const Scalar theta = theta0 * clampedT;
        const Scalar sinTheta = std::sin(theta);
        const Scalar s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
        const Scalar s1 = sinTheta / sinTheta0;

        return Quaternion{
            s0 * q1.x + s1 * q2.x,
            s0 * q1.y + s1 * q2.y,
            s0 * q1.z + s1 * q2.z,
            s0 * q1.w + s1 * q2.w}.normalized();
    }

    [[nodiscard]] bool isFinite() const noexcept
    {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w);
    }

    [[nodiscard]] bool isNormalized(Scalar epsilon = Scalar(1e-12)) const noexcept
    {
        return std::abs(lengthSquared() - Scalar(1.0)) <= epsilon;
    }

    friend constexpr bool operator==(const Quaternion& a, const Quaternion& b) noexcept
    {
        return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
    }

    friend constexpr bool operator!=(const Quaternion& a, const Quaternion& b) noexcept
    {
        return !(a == b);
    }
};

} // namespace mir
