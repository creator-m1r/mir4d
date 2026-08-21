#pragma once

#include "Time.hpp"

namespace mir
{

/// Deterministic simulation clock for the 4D engine.
/// One tick is the engine's elementary simulation step; the default is one second.
class Clock
{
public:
    explicit constexpr Clock(double tickPeriodSeconds = 1.0) noexcept
        : tickPeriodSeconds_(tickPeriodSeconds > 0.0 ? tickPeriodSeconds : 1.0)
    {
    }

    [[nodiscard]] constexpr Time time() const noexcept
    {
        return time_;
    }

    [[nodiscard]] constexpr double tickPeriod() const noexcept
    {
        return tickPeriodSeconds_;
    }

    constexpr void reset() noexcept
    {
        time_.setSeconds(0.0);
    }

    constexpr void setTime(Time time) noexcept
    {
        if (time.isValid())
            time_ = time;
    }

    constexpr void tick() noexcept
    {
        time_.advance(tickPeriodSeconds_);
    }

    constexpr void advance(double deltaSeconds) noexcept
    {
        if (deltaSeconds > 0.0)
            time_.advance(deltaSeconds);
    }

private:
    Time time_{};
    double tickPeriodSeconds_{1.0};
};

} // namespace mir
