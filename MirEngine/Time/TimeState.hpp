#pragma once

#include "Time.hpp"

namespace mir
{

/// Time state shared by future physical and process simulations.
/// Geometry itself remains timeless; simulation systems use this state to
/// evaluate material, kinematic and process changes at a given instant.
struct TimeState
{
    Time current{};
    double deltaSeconds{0.0};
    Time::Tick tickIndex{0};

    [[nodiscard]] bool isValid() const noexcept
    {
        return current.isValid() && deltaSeconds >= 0.0;
    }
};

} // namespace mir
