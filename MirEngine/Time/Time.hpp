#pragma once

#include <cmath>
#include <cstdint>

namespace mir4d
{

class Time
{
public:
    using Tick = std::uint64_t;

    constexpr Time() noexcept = default;

    explicit constexpr Time(double seconds) noexcept
        : seconds_(seconds >= 0.0 ? seconds : 0.0)
    {
    }

    [[nodiscard]] constexpr double seconds() const noexcept
    {
        return seconds_;
    }

    [[nodiscard]] constexpr Tick tick(double tickPeriod = 1.0) const noexcept
    {
        if (!(tickPeriod > 0.0) || !std::isfinite(tickPeriod))
            return 0;
        return static_cast<Tick>(seconds_ / tickPeriod);
    }

    constexpr void setSeconds(double seconds) noexcept
    {
        if (seconds >= 0.0 && std::isfinite(seconds))
            seconds_ = seconds;
    }

    constexpr void advance(double deltaSeconds) noexcept
    {
        if (deltaSeconds >= 0.0 && std::isfinite(deltaSeconds))
            seconds_ += deltaSeconds;
    }

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return seconds_ >= 0.0 && std::isfinite(seconds_);
    }

private:
    double seconds_{0.0};
};

}
