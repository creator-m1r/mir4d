#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Math/Vector/Vector3.hpp"

#include <cmath>
#include <cstdint>

namespace mir
{

enum class MechanismType
{
    Generic,
    Motor,
    Shaft,
    Gear,
    GearPair,
    Bearing,
    Belt,
    Chain,
    Pulley,
    Joint,
    Drive,
    Tool
};

struct AngularState
{
    Scalar position{0.0};
    Scalar velocity{0.0};
    Scalar acceleration{0.0};
    Scalar torque{0.0};

    [[nodiscard]] bool isValid() const noexcept
    {
        return std::isfinite(position) &&
               std::isfinite(velocity) &&
               std::isfinite(acceleration) &&
               std::isfinite(torque);
    }
};

struct LinearState
{
    Vector3 position{};
    Vector3 velocity{};
    Vector3 acceleration{};
    Vector3 force{};

    [[nodiscard]] bool isValid() const noexcept
    {
        return position.isFinite() && velocity.isFinite() &&
               acceleration.isFinite() && force.isFinite();
    }
};

struct GearData
{
    std::uint32_t teeth{1};
    Scalar pitchDiameter{1.0};

    [[nodiscard]] bool isValid() const noexcept
    {
        return teeth > 0 && std::isfinite(pitchDiameter) && pitchDiameter > 0.0;
    }
};

struct GearPairData
{
    GearData input{};
    GearData output{};

    [[nodiscard]] Scalar ratio() const noexcept
    {
        if (output.teeth == 0)
            return 0.0;
        return static_cast<Scalar>(input.teeth) /
               static_cast<Scalar>(output.teeth);
    }

    [[nodiscard]] Scalar outputVelocity(Scalar inputVelocity) const noexcept
    {
        return -inputVelocity * ratio();
    }

    [[nodiscard]] Scalar outputTorque(Scalar inputTorque) const noexcept
    {
        if (ratio() == 0.0)
            return 0.0;
        return inputTorque / ratio();
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        return input.isValid() && output.isValid();
    }
};

struct MotorData
{
    Scalar maxSpeed{0.0};
    Scalar maxTorque{0.0};
    Scalar targetSpeed{0.0};
    bool enabled{false};

    [[nodiscard]] bool isValid() const noexcept
    {
        return std::isfinite(maxSpeed) && std::isfinite(maxTorque) &&
               std::isfinite(targetSpeed) && maxSpeed >= 0.0 && maxTorque >= 0.0;
    }
};

struct ShaftData
{
    Scalar length{1.0};
    Scalar radius{0.01};
    Scalar inertia{1.0};
    AngularState angular{};

    [[nodiscard]] bool isValid() const noexcept
    {
        return std::isfinite(length) && std::isfinite(radius) &&
               std::isfinite(inertia) && length > 0.0 && radius > 0.0 &&
               inertia > 0.0 && angular.isValid();
    }
};

} // namespace mir