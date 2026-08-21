#pragma once

#include "Camera.hpp"
#include "MirEngine/Math/Point.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

/// Platform-neutral camera interaction controller.
/// Middle-button orbit/pan/zoom policies are intentionally configurable by the UI.
///
/// Industrial CAD navigation semantics:
///   - orbit: turntable around the target (yaw/pitch clamped);
///   - pan:   moves the target inside the camera view plane (screen X/Y),
///            so panning never "drifts" relative to the view orientation;
///   - zoom:  exponential dolly, optionally anchored to the point under the
///            cursor (perspective keeps the cursor point fixed on the screen;
///            orthographic rescales the visible volume around it).
class ViewportController
{
public:
    explicit ViewportController(Camera* camera = nullptr) noexcept
        : camera_(camera)
    {
    }

    void setCamera(Camera* camera) noexcept { camera_ = camera; }
    [[nodiscard]] Camera* camera() const noexcept { return camera_; }

    void setOrbitSpeed(Scalar value) noexcept { orbitSpeed_ = std::max(value, Scalar(0)); }
    void setPanSpeed(Scalar value) noexcept { panSpeed_ = std::max(value, Scalar(0)); }
    void setZoomSpeed(Scalar value) noexcept { zoomSpeed_ = std::max(value, Scalar(0)); }

    void beginOrbit(Scalar x, Scalar y) noexcept
    {
        begin(x, y);
        mode_ = Mode::Orbit;
    }

    void beginPan(Scalar x, Scalar y) noexcept
    {
        begin(x, y);
        mode_ = Mode::Pan;
    }

    void end() noexcept { mode_ = Mode::None; }

    void move(Scalar x, Scalar y) noexcept
    {
        if (!camera_ || mode_ == Mode::None)
            return;

        const Scalar dx = x - lastX_;
        const Scalar dy = y - lastY_;
        lastX_ = x;
        lastY_ = y;

        if (mode_ == Mode::Orbit)
        {
            camera_->setOrbit(
                camera_->theta() - dx * orbitSpeed_,
                clampPhi(camera_->phi() - dy * orbitSpeed_),
                camera_->distance());
            return;
        }

        panBy(dx, dy);
    }

    void zoom(Scalar delta) noexcept
    {
        if (!camera_)
            return;

        const Scalar factor = std::exp(-delta * zoomSpeed_);
        camera_->setOrbit(camera_->theta(), camera_->phi(), camera_->distance() * factor);
    }

    /// Continuous two-finger / gesture panning.
    /// Deltas follow the touchpad gesture direction, independent of Mode,
    /// so trackpad panning never conflicts with mouse button state.
    /// Pan is performed in the camera view plane (right/up vectors), so the
    /// scene follows the pointer regardless of the orbit orientation.
    void panBy(Scalar dx, Scalar dy) noexcept
    {
        if (!camera_)
            return;

        const Point3 target = camera_->target();
        const Vector3 right = camera_->rightVector();
        const Vector3 up = camera_->upVector();
        const Scalar scale = camera_->distance() * panSpeed_;

        camera_->setTarget({
            target.x + (-right.x * dx + up.x * dy) * scale,
            target.y + (-right.y * dx + up.y * dy) * scale,
            target.z + (-right.z * dx + up.z * dy) * scale});
    }

    /// Continuous two-finger / gesture orbiting.
    /// Deltas follow the touchpad gesture direction, independent of Mode,
    /// so trackpad orbiting never conflicts with mouse button state.
    void orbitBy(Scalar dx, Scalar dy) noexcept
    {
        if (!camera_)
            return;

        camera_->setOrbit(
            camera_->theta() - dx * orbitSpeed_,
            clampPhi(camera_->phi() - dy * orbitSpeed_),
            camera_->distance());
    }

    /// Exponential zoom anchored at the world point hit by the picking ray.
    ///
    /// rayOrigin/rayDirection describe the ray from the eye through the cursor
    /// pixel (see RayPicker::buildRay). The cursor point stays under the
    /// pointer: perspective adjusts the target along the ray; orthographic
    /// rescales the visible volume around the cursor point.
    void zoomAt(Scalar delta,
                const Point3& rayOrigin,
                const Vector3& rayDirection) noexcept
    {
        if (!camera_)
            return;

        const Scalar d0 = camera_->distance();
        const Scalar d1 = std::max(d0 * std::exp(-delta * zoomSpeed_), Scalar(1e-6));
        if (std::abs(d1 - d0) <= Scalar(1e-12))
            return;

        const Vector3 forward = camera_->forward();
        const Scalar cosAngle = std::max(Vector3::dot(rayDirection, forward), Scalar(1e-6));

        // P: intersection of the picking ray with the plane through the
        // target perpendicular to the view direction. rayOrigin lies on the
        // near clip plane (not at the eye), so the parameterization uses the
        // target-relative distance along the ray.
        const Point3 target = camera_->target();
        const Scalar t0 = (Vector3{target.x - rayOrigin.x,
                                   target.y - rayOrigin.y,
                                   target.z - rayOrigin.z}.dot(forward)) /
                          cosAngle;
        const Point3 pivot{
            rayOrigin.x + rayDirection.x * t0,
            rayOrigin.y + rayDirection.y * t0,
            rayOrigin.z + rayDirection.z * t0};

        Point3 newTarget;
        if (camera_->projection() == CameraProjection::Perspective)
        {
            const Scalar t1 = d1 / cosAngle;
            newTarget = {
                pivot.x - rayDirection.x * t1 + forward.x * d1,
                pivot.y - rayDirection.y * t1 + forward.y * d1,
                pivot.z - rayDirection.z * t1 + forward.z * d1};
        }
        else
        {
            const Scalar ratio = d1 / d0;
            newTarget = {
                pivot.x + (target.x - pivot.x) * ratio,
                pivot.y + (target.y - pivot.y) * ratio,
                pivot.z + (target.z - pivot.z) * ratio};
        }

        camera_->setOrbit(camera_->theta(), camera_->phi(), d1);
        camera_->setTarget(newTarget);
    }

private:
    enum class Mode { None, Orbit, Pan };

    void begin(Scalar x, Scalar y) noexcept
    {
        lastX_ = x;
        lastY_ = y;
    }

    static Scalar clampPhi(Scalar phi) noexcept
    {
        return std::clamp(phi, kMinPhi, kMaxPhi);
    }

    Camera* camera_{nullptr};
    Mode mode_{Mode::None};
    Scalar lastX_{0};
    Scalar lastY_{0};
    Scalar orbitSpeed_{0.005};
    Scalar panSpeed_{0.015};
    Scalar zoomSpeed_{0.8};

    static constexpr Scalar kMinPhi = Scalar(0.05);
    static constexpr Scalar kMaxPhi = Scalar(3.14159265358979) - Scalar(0.05);
};

} // namespace mir