#pragma once

#include "MirEngine/Core/Types/Scalar.hpp"
#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/Vector/Vector.hpp"
#include "MirEngine/Math/TransformMatrix.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mir
{

enum class CameraProjection
{
    Perspective,
    Orthographic
};

/// Canonical camera value used by Viewport and future renderer backends.
/// No raw float vector or matrix types are exposed here.
class Camera
{
public:
    Camera() = default;

    void setPerspective(Scalar fovYRadians,
                        Scalar aspect,
                        Scalar nearPlane,
                        Scalar farPlane) noexcept
    {
        projection_ = CameraProjection::Perspective;
        fovY_ = fovYRadians;
        aspect_ = std::max(aspect, Scalar(1e-12));
        nearPlane_ = std::max(nearPlane, Scalar(1e-12));
        farPlane_ = std::max(farPlane, nearPlane_ + Scalar(1e-9));
        projectionDirty_ = true;
    }

    void setOrthographic(Scalar left,
                         Scalar right,
                         Scalar bottom,
                         Scalar top,
                         Scalar nearPlane,
                         Scalar farPlane) noexcept
    {
        projection_ = CameraProjection::Orthographic;
        orthoLeft_ = left;
        orthoRight_ = right;
        orthoBottom_ = bottom;
        orthoTop_ = top;
        nearPlane_ = nearPlane;
        farPlane_ = std::max(farPlane, nearPlane_ + Scalar(1e-9));
        projectionDirty_ = true;
    }

    void setAspect(Scalar aspect) noexcept
    {
        aspect_ = std::max(aspect, Scalar(1e-12));
        projectionDirty_ = true;
    }

    void setTarget(const Point3& target) noexcept
    {
        target_ = target;
        viewDirty_ = true;
    }

    void setOrbit(Scalar theta, Scalar phi, Scalar distance) noexcept
    {
        theta_ = theta;
        phi_ = std::clamp(phi, Scalar(1e-5), Scalar(3.1415926535897932384626433832795 - 1e-5));
        distance_ = std::max(distance, Scalar(1e-9));
        viewDirty_ = true;
    }

    [[nodiscard]] Point3 target() const noexcept { return target_; }
    [[nodiscard]] Scalar theta() const noexcept { return theta_; }
    [[nodiscard]] Scalar phi() const noexcept { return phi_; }
    [[nodiscard]] Scalar distance() const noexcept { return distance_; }
    [[nodiscard]] CameraProjection projection() const noexcept { return projection_; }
    [[nodiscard]] Scalar fovY() const noexcept { return fovY_; }
    [[nodiscard]] Scalar nearPlane() const noexcept { return nearPlane_; }
    [[nodiscard]] Scalar farPlane() const noexcept { return farPlane_; }

    [[nodiscard]] Point3 position() const noexcept
    {
        const Scalar sinPhi = std::sin(phi_);
        const Scalar cosPhi = std::cos(phi_);
        const Scalar sinTheta = std::sin(theta_);
        const Scalar cosTheta = std::cos(theta_);

        return {
            target_.x + distance_ * sinPhi * sinTheta,
            target_.y + distance_ * cosPhi,
            target_.z + distance_ * sinPhi * cosTheta};
    }

    [[nodiscard]] Matrix4 viewMatrix() const noexcept
    {
        const Point3 eye = position();
        const Vector3 forward = Vector3{
            target_.x - eye.x,
            target_.y - eye.y,
            target_.z - eye.z}.normalized();

        Vector3 up{0.0, 1.0, 0.0};
        if (std::abs(Vector3::dot(forward, up)) > Scalar(0.9999))
            up = {0.0, 0.0, 1.0};

        const Vector3 right = Vector3::cross(forward, up).normalized();
        const Vector3 correctedUp = Vector3::cross(right, forward).normalized();

        return Matrix4{
            right.x, right.y, right.z,
            -Vector3::dot(right, Vector3{eye.x, eye.y, eye.z}),
            correctedUp.x, correctedUp.y, correctedUp.z,
            -Vector3::dot(correctedUp, Vector3{eye.x, eye.y, eye.z}),
            -forward.x, -forward.y, -forward.z,
            Vector3::dot(forward, Vector3{eye.x, eye.y, eye.z}),
            0.0, 0.0, 0.0, 1.0};
    }

    [[nodiscard]] Matrix4 projectionMatrix() const noexcept
    {
        if (projection_ == CameraProjection::Perspective)
        {
            const Scalar f = Scalar(1.0) / std::tan(fovY_ * Scalar(0.5));
            const Scalar nf = Scalar(1.0) / (nearPlane_ - farPlane_);
            return Matrix4{
                f / aspect_, 0.0, 0.0, 0.0,
                0.0, f, 0.0, 0.0,
                0.0, 0.0, (farPlane_ + nearPlane_) * nf,
                Scalar(2.0) * farPlane_ * nearPlane_ * nf,
                0.0, 0.0, -1.0, 0.0};
        }

        const Scalar rl = orthoRight_ - orthoLeft_;
        const Scalar tb = orthoTop_ - orthoBottom_;
        const Scalar fn = farPlane_ - nearPlane_;
        return Matrix4{
            Scalar(2.0) / rl, 0.0, 0.0, -(orthoRight_ + orthoLeft_) / rl,
            0.0, Scalar(2.0) / tb, 0.0, -(orthoTop_ + orthoBottom_) / tb,
            0.0, 0.0, -Scalar(2.0) / fn, -(farPlane_ + nearPlane_) / fn,
            0.0, 0.0, 0.0, 1.0};
    }

private:
    CameraProjection projection_{CameraProjection::Perspective};
    Point3 target_{};
    Scalar theta_{0.8};
    Scalar phi_{1.2};
    Scalar distance_{12.0};
    Scalar fovY_{0.7853981633974483};
    Scalar aspect_{1.0};
    Scalar nearPlane_{0.1};
    Scalar farPlane_{500.0};
    Scalar orthoLeft_{-10.0};
    Scalar orthoRight_{10.0};
    Scalar orthoBottom_{-10.0};
    Scalar orthoTop_{10.0};
    bool viewDirty_{true};
    bool projectionDirty_{true};
};

} // namespace mir
