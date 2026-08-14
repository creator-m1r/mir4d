#pragma once

#include "Camera.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

/// Platform-neutral camera interaction controller.
/// Middle-button orbit/pan/zoom policies are intentionally configurable by the UI.
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

        const Point3 target = camera_->target();
        const Scalar scale = camera_->distance() * panSpeed_;
        camera_->setTarget({target.x - dx * scale, target.y + dy * scale, target.z});
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
    void panBy(Scalar dx, Scalar dy) noexcept
    {
        if (!camera_)
            return;

        const Point3 target = camera_->target();
        const Scalar scale = camera_->distance() * panSpeed_;
        camera_->setTarget({target.x - dx * scale, target.y + dy * scale, target.z});
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
