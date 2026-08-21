#pragma once

#include "Time.hpp"

namespace mir
{

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

}
