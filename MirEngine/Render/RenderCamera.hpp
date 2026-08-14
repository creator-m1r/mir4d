#pragma once

#include "RenderTypes.hpp"

#include <cmath>

namespace mir
{

class RenderCamera
{
public:
    void setTarget(RenderVec3 target) noexcept { target_ = target; }
    void moveTarget(RenderVec3 delta) noexcept
    {
        target_.x += delta.x;
        target_.y += delta.y;
        target_.z += delta.z;
    }
    void setDistance(double distance) noexcept { distance_ = distance > 1e-6 ? distance : 1e-6; }
    void setYaw(double radians) noexcept { yaw_ = radians; }
    void setPitch(double radians) noexcept
    {
        constexpr double limit = 1.5533430342749532;
        pitch_ = radians < -limit ? -limit : (radians > limit ? limit : radians);
    }
    void setPerspective(double fovRadians, double aspect, double nearPlane, double farPlane) noexcept;

    [[nodiscard]] RenderVec3 target() const noexcept { return target_; }
    [[nodiscard]] double distance() const noexcept { return distance_; }
    [[nodiscard]] double yaw() const noexcept { return yaw_; }
    [[nodiscard]] double pitch() const noexcept { return pitch_; }
    [[nodiscard]] RenderVec3 position() const noexcept;
    [[nodiscard]] RenderMat4 viewMatrix() const noexcept;
    [[nodiscard]] RenderMat4 projectionMatrix() const noexcept;

private:
    RenderVec3 target_{0.0, 0.0, 0.0};
    double distance_{5.0};
    double yaw_{0.0};
    double pitch_{0.35};
    double fov_{1.0471975511965976};
    double aspect_{1.0};
    double nearPlane_{0.01};
    double farPlane_{10000.0};
};

} // namespace mir
