#pragma once

#include "ProcessTypes.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

enum class MechanicalQuality
{
    Fast,
    Balanced,
    High
};

struct MechanicalState
{
    Scalar angularPosition{0.0};
    Scalar angularVelocity{0.0};
    Scalar torque{0.0};
    Scalar inertia{1.0};
    Scalar damping{0.0};
    Scalar loadTorque{0.0};
    bool enabled{true};
};

class MechanicalSolver
{
public:
    void setQuality(MechanicalQuality quality) noexcept { quality_ = quality; }
    [[nodiscard]] MechanicalQuality quality() const noexcept { return quality_; }

    void solve(MechanicalState& state, Scalar deltaSeconds) const noexcept
    {
        if (!state.enabled || deltaSeconds <= 0.0)
            return;

        const Scalar inertia = std::max(0.000001, state.inertia);
        const Scalar damping = std::max(0.0, state.damping);
        const Scalar acceleration = (state.torque - state.loadTorque - damping * state.angularVelocity) / inertia;
        const Scalar response = responseFactor();

        state.angularVelocity += acceleration * deltaSeconds * response;
        state.angularPosition += state.angularVelocity * deltaSeconds * response;

        if (std::abs(state.angularVelocity) < 1e-9)
            state.angularVelocity = 0.0;
    }

private:
    [[nodiscard]] Scalar responseFactor() const noexcept
    {
        switch (quality_)
        {
            case MechanicalQuality::Fast:     return 1.0;
            case MechanicalQuality::Balanced: return 0.5;
            case MechanicalQuality::High:     return 0.25;
        }
        return 1.0;
    }

    MechanicalQuality quality_{MechanicalQuality::Fast};
};

struct MechanicalTransmission
{
    Scalar inputRadius{1.0};
    Scalar outputRadius{1.0};

    [[nodiscard]] Scalar ratio() const noexcept
    {
        return outputRadius <= 0.0 ? 1.0 : inputRadius / outputRadius;
    }

    void transfer(const MechanicalState& input, MechanicalState& output) const noexcept
    {
        output.angularVelocity = input.angularVelocity * ratio();
        output.torque = input.torque / std::max(0.000001, ratio());
    }
};

}
