#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace mir
{

struct SimulationPriorityField
{
    WorldPosition center{};
    double influenceRadius{32.0};
    std::uint8_t basePriority{1};
    std::uint8_t maximumPriority{10};

    [[nodiscard]] std::uint8_t priorityAt(WorldPosition position) const noexcept
    {
        const double dx = position.x - center.x;
        const double dy = position.y - center.y;
        const double dz = position.z - center.z;
        const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (distance >= influenceRadius)
            return basePriority;

        const double normalized = 1.0 - distance / influenceRadius;
        const double smooth = normalized * normalized * (3.0 - 2.0 * normalized);
        const double value = static_cast<double>(basePriority) +
            smooth * static_cast<double>(maximumPriority - basePriority);

        return static_cast<std::uint8_t>(std::clamp(
            value,
            static_cast<double>(basePriority),
            static_cast<double>(maximumPriority)));
    }
};

} // namespace mir
