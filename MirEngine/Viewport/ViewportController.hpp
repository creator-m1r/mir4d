#pragma once

#include "Camera.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

/// Platform-neutral CAD camera controller.
/// Pan follows the camera screen axes instead of world X/Y, while orbit and
/// zoom remain stable across very small and very large engineering scenes.
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

        panCameraAxes(dx, dy);
    }

    void zoom(Scalar delta) noexcept
    {
        if (!camera_)
            return;

        // Exponential zoom gives the same feel at any scene scale.
        const Scalar factor = std::exp(-delta * zoomSpeed_);
        const Scalar distance = std::clamp(
            camera_->distance() * factor,
            kMinDistance,
            kMaxDistance);
        camera_->setOrbit(camera_->theta(), camera_->phi(), distance);
    }

    void panBy(Scalar dx, Scalar dy) noexcept
    {
        if (!camera_)
            return;
        panCameraAxes(dx, dy);
    }

    void orbitBy(Scalar dx, Scalar dy) noexcept
    {
        if (!camera_)
            return;

        camera_->setOrbit(
            camera_->theta() - dx * orbitSpeed_,
            clampPhi(camera_->phi() - dy * orbitSpeed_),
            camera_->distance());
    }

private:
    enum class Mode { None, Orbit, Pan };

    void begin(Scalar x, Scalar y) noexcept
    {
        lastX_ = x;
        lastY_ = y;
    }

    void panCameraAxes(Scalar dx, Scalar dy) noexcept
    {
        const Point3 target = camera_->target();
        const Scalar distance = std::max(camera_->distance(), Scalar(1e-6));
        const Scalar scale = distance * panSpeed_;

        // Camera position uses:
        // x = sin(phi) sin(theta), y = cos(phi), z = sin(phi) cos(theta).
        // The screen-right vector is perpendicular to the horizontal view
        // direction; screen-up is the camera's corrected up vector.
        const Scalar theta = camera_->theta();
        const Scalar phi = camera_->phi();
        const Scalar sinTheta = std::sin(theta);
        const Scalar cosTheta = std::cos(theta);
        const Scalar sinPhi = std::sin(phi);
        const Scalar cosPhi = std::cos(phi);

        const Vector3 right{cosTheta, 0.0, -sinTheta};
        const Vector3 up{-cosPhi * sinTheta, sinPhi, -cosPhi * cosTheta};

        const Scalar tx = -dx * scale;
        const Scalar ty = dy * scale;
        camera_->setTarget({
            target.x + right.x * tx + up.x * ty,
            target.y + right.y * tx + up.y * ty,
            target.z + right.z * tx + up.z * ty});
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
    static constexpr Scalar kMinDistance = Scalar(1e-6);
    static constexpr Scalar kMaxDistance = Scalar(1e12);
};

} // namespace mir
