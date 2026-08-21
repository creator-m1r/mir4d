#pragma once

#include "ProcessTypes.hpp"

#include <algorithm>

namespace mir
{

class SimulationClock
{
public:
    void reset() noexcept
    {
        time_ = 0.0;
        deltaTime_ = 0.0;
        running_ = false;
    }

    void start() noexcept { running_ = true; }
    void pause() noexcept { running_ = false; }
    void toggle() noexcept { running_ = !running_; }

    void setTime(Scalar time) noexcept { time_ = std::max(0.0, time); }
    void setTimeScale(Scalar scale) noexcept { timeScale_ = std::max(0.0, scale); }

    [[nodiscard]] Scalar time() const noexcept { return time_; }
    [[nodiscard]] Scalar deltaTime() const noexcept { return deltaTime_; }
    [[nodiscard]] Scalar timeScale() const noexcept { return timeScale_; }
    [[nodiscard]] bool running() const noexcept { return running_; }

    void tick(Scalar realDeltaSeconds) noexcept
    {
        if (!running_ || realDeltaSeconds <= 0.0)
        {
            deltaTime_ = 0.0;
            return;
        }

        deltaTime_ = realDeltaSeconds * timeScale_;
        time_ += deltaTime_;
    }

private:
    Scalar time_{0.0};
    Scalar deltaTime_{0.0};
    Scalar timeScale_{1.0};
    bool running_{false};
};

}
