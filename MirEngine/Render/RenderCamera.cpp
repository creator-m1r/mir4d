#include "RenderCamera.hpp"

namespace mir
{

void RenderCamera::setPerspective(
    double fovRadians,
    double aspect,
    double nearPlane,
    double farPlane) noexcept
{
    fov_ = fovRadians > 1e-6 ? fovRadians : 1e-6;
    aspect_ = aspect > 1e-6 ? aspect : 1.0;
    nearPlane_ = nearPlane > 1e-6 ? nearPlane : 1e-6;
    farPlane_ = farPlane > nearPlane_ ? farPlane : nearPlane_ + 1.0;
}

RenderVec3 RenderCamera::position() const noexcept
{
    const double cosPitch = std::cos(pitch_);
    return {
        target_.x + distance_ * cosPitch * std::sin(yaw_),
        target_.y + distance_ * std::sin(pitch_),
        target_.z + distance_ * cosPitch * std::cos(yaw_)};
}

RenderMat4 RenderCamera::viewMatrix() const noexcept
{
    const auto eye = position();
    const RenderVec3 forward{
        target_.x - eye.x,
        target_.y - eye.y,
        target_.z - eye.z};

    const double length = std::sqrt(
        forward.x * forward.x +
        forward.y * forward.y +
        forward.z * forward.z);

    if (length <= 1e-12)
        return RenderMat4::identity();

    const RenderVec3 f{
        forward.x / length,
        forward.y / length,
        forward.z / length};

    RenderVec3 up{0.0, 1.0, 0.0};
    RenderVec3 s{
        f.y * up.z - f.z * up.y,
        f.z * up.x - f.x * up.z,
        f.x * up.y - f.y * up.x};

    const double sLength = std::sqrt(s.x * s.x + s.y * s.y + s.z * s.z);
    if (sLength <= 1e-12)
        return RenderMat4::identity();

    s = {s.x / sLength, s.y / sLength, s.z / sLength};

    const RenderVec3 u{
        s.y * f.z - s.z * f.y,
        s.z * f.x - s.x * f.z,
        s.x * f.y - s.y * f.x};

    RenderMat4 result = RenderMat4::identity();
    result.m[0] = s.x;
    result.m[4] = s.y;
    result.m[8] = s.z;
    result.m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);

    result.m[1] = u.x;
    result.m[5] = u.y;
    result.m[9] = u.z;
    result.m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);

    result.m[2] = -f.x;
    result.m[6] = -f.y;
    result.m[10] = -f.z;
    result.m[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;

    return result;
}

RenderMat4 RenderCamera::projectionMatrix() const noexcept
{
    const double tanHalfFov = std::tan(fov_ * 0.5);
    if (std::abs(tanHalfFov) <= 1e-12)
        return RenderMat4::identity();

    RenderMat4 result{};
    result.m[0] = 1.0 / (aspect_ * tanHalfFov);
    result.m[5] = 1.0 / tanHalfFov;
    result.m[10] = -(farPlane_ + nearPlane_) / (farPlane_ - nearPlane_);
    result.m[11] = -1.0;
    result.m[14] = -(2.0 * farPlane_ * nearPlane_) / (farPlane_ - nearPlane_);
    return result;
}

} // namespace mir
