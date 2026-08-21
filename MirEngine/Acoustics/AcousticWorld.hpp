#pragma once

#include "AcousticsTypes.hpp"

#include <vector>

namespace mir
{

class AcousticWorld
{
public:
    void setSettings(const AcousticSettings& settings) noexcept { settings_ = settings; }
    [[nodiscard]] const AcousticSettings& settings() const noexcept { return settings_; }

    void emit(const AcousticEvent& event)
    {
        if (!settings_.enabled || event.intensity <= 0.0)
            return;
        events_.push_back(event);
    }

    [[nodiscard]] const std::vector<AcousticEvent>& events() const noexcept
    {
        return events_;
    }

    void clear() noexcept { events_.clear(); }

private:
    AcousticSettings settings_{};
    std::vector<AcousticEvent> events_{};
};

} // namespace mir
