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
///
/// Orbit model: the camera orbits a target point at (theta, phi, distance).
/// The view direction is always target - position, so orbit never rolls.
/// Orthographic extents are derived from the orbit distance (half-height =
/// distance, half-width = distance * aspect), so zooming keeps working in
/// orthographic mode and switching projections preserves the visible scale.
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
        fovY_ = std::max(fovYRadians, Scalar(1e-6));
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

    void setProjection(CameraProjection projection) noexcept
    {
        projection_ = projection;
        projectionDirty_ = true;
    }

    void setAspect(Scalar aspect) noexcept
    {
        aspect_ = std::max(aspect, Scalar(1e-12));
        projectionDirty_ = true;
    }

    /// Updates near/far planes without touching the projection mode.
    /// Used by the runtime to keep dynamic clipping in orthographic mode.
    void setNearFar(Scalar nearPlane, Scalar farPlane) noexcept
    {
        nearPlane_ = std::max(nearPlane, Scalar(1e-12));
        farPlane_ = std::max(farPlane, nearPlane_ + Scalar(1e-9));
        projectionDirty_ = true;
    }

    void setFovY(Scalar fovYRadians) noexcept
    {
        fovY_ = std::clamp(fovYRadians, Scalar(0.017453292519943295), Scalar(1.7453292519943295));
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
            target_.y + distance_ * sinPhi * cosTheta,
            target_.z + distance_ * cosPhi};
    }

    /// World-space view direction: from the eye towards the target.
    [[nodiscard]] Vector3 forward() const noexcept
    {
        const Point3 eye = position();
        return Vector3{
            target_.x - eye.x,
            target_.y - eye.y,
            target_.z - eye.z}.normalized();
    }

    /// World-space right vector of the view plane (screen X).
    [[nodiscard]] Vector3 rightVector() const noexcept
    {
        const Vector3 f = forward();
        // Z-up world: the default up is +Z; fall back to +Y when looking
        // straight down the Z axis (degenerate up).
        Vector3 worldUp{0.0, 0.0, 1.0};
        if (std::abs(Vector3::dot(f, worldUp)) > Scalar(0.9999))
            worldUp = {0.0, 1.0, 0.0};
        return Vector3::cross(f, worldUp).normalized();
    }

    /// World-space up vector of the view plane (screen Y).
    [[nodiscard]] Vector3 upVector() const noexcept
    {
        return Vector3::cross(rightVector(), forward()).normalized();
    }

    /// Orthographic half-height of the visible volume (derived from distance).
    [[nodiscard]] Scalar orthoHalfHeight() const noexcept { return distance_; }
    /// Orthographic half-width of the visible volume (aspect-scaled distance).
    [[nodiscard]] Scalar orthoHalfWidth() const noexcept { return distance_ * aspect_; }

    [[nodiscard]] Matrix4 viewMatrix() const noexcept
    {
        const Point3 eye = position();
        const Vector3 f = forward();
        const Vector3 r = rightVector();
        const Vector3 u = upVector();

        return Matrix4{
            r.x, r.y, r.z,
            -Vector3::dot(r, Vector3{eye.x, eye.y, eye.z}),
            u.x, u.y, u.z,
            -Vector3::dot(u, Vector3{eye.x, eye.y, eye.z}),
            -f.x, -f.y, -f.z,
            Vector3::dot(f, Vector3{eye.x, eye.y, eye.z}),
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

        // Orthographic extents follow the orbit distance so zoom keeps the
        // visible scale intuitive: half-height = distance, half-width =
        // distance * aspect. Explicit setOrthographic values remain stored but
        // the runtime keeps them derived while zooming.
        const Scalar halfHeight = distance_;
        const Scalar halfWidth = distance_ * aspect_;
        const Scalar left = orthoLeft_ == Scalar(0.0) ? -halfWidth : orthoLeft_;
        const Scalar right = orthoRight_ == Scalar(0.0) ? halfWidth : orthoRight_;
        const Scalar bottom = orthoBottom_ == Scalar(0.0) ? -halfHeight : orthoBottom_;
        const Scalar top = orthoTop_ == Scalar(0.0) ? halfHeight : orthoTop_;

        const Scalar rl = right - left;
        const Scalar tb = top - bottom;
        const Scalar fn = farPlane_ - nearPlane_;
        return Matrix4{
            Scalar(2.0) / rl, 0.0, 0.0, -(right + left) / rl,
            0.0, Scalar(2.0) / tb, 0.0, -(top + bottom) / tb,
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
    Scalar orthoLeft_{0.0};
    Scalar orthoRight_{0.0};
    Scalar orthoBottom_{0.0};
    Scalar orthoTop_{0.0};
    bool viewDirty_{true};
    bool projectionDirty_{true};
};

} // namespace mir