#pragma once

#include "../../World/Spatial/SpatialWorldPartition.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>

namespace mir
{

struct InterestCamera
{
    WorldPosition position{};
    double forwardX{0.0};
    double forwardY{0.0};
    double forwardZ{1.0};
};

struct WorldInterestSource
{
    WorldPosition position{};
    double radius{16.0};
    float weight{1.0F};
    bool selected{false};
    bool simulationActive{false};
    bool aiFocused{false};
};

struct WorldInterestScore
{
    float score{0.0F};
    std::uint8_t priority{0};
    bool visible{false};
    bool important{false};
};

class WorldInterestManager
{
public:
    void setCamera(const InterestCamera& camera) noexcept
    {
        camera_ = camera;
    }

    void setSource(std::uint64_t objectId, const WorldInterestSource& source)
    {
        sources_[objectId] = source;
    }

    void removeSource(std::uint64_t objectId)
    {
        sources_.erase(objectId);
    }

    [[nodiscard]] WorldInterestScore evaluate(std::uint64_t objectId) const noexcept
    {
        const auto iterator = sources_.find(objectId);
        if (iterator == sources_.end())
            return {};

        const auto& source = iterator->second;
        const double dx = source.position.x - camera_.position.x;
        const double dy = source.position.y - camera_.position.y;
        const double dz = source.position.z - camera_.position.z;
        const double distanceSquared = dx * dx + dy * dy + dz * dz;
        const double distance = std::sqrt(distanceSquared);

        const double safeRadius = std::max(0.001, source.radius);
        const float proximity = static_cast<float>(
            std::clamp(1.0 - distance / safeRadius, 0.0, 1.0));

        const double length = std::sqrt(
            camera_.forwardX * camera_.forwardX +
            camera_.forwardY * camera_.forwardY +
            camera_.forwardZ * camera_.forwardZ);

        const double directionLength = std::max(0.001, length);
        const double viewDot = (dx * camera_.forwardX +
                                dy * camera_.forwardY +
                                dz * camera_.forwardZ) /
            (std::max(0.001, distance) * directionLength);

        const float viewFactor = static_cast<float>(
            std::clamp((viewDot + 1.0) * 0.5, 0.0, 1.0));

        float score = proximity * 0.45F + viewFactor * 0.15F;
        score += source.weight * 0.10F;
        score += source.selected ? 0.20F : 0.0F;
        score += source.simulationActive ? 0.20F : 0.0F;
        score += source.aiFocused ? 0.30F : 0.0F;
        score = std::clamp(score, 0.0F, 1.0F);

        const auto priority = static_cast<std::uint8_t>(
            std::clamp(std::lround(score * 10.0F), 0L, 10L));

        return {
            score,
            priority,
            distance <= safeRadius * 2.0,
            priority >= 7};
    }

private:
    InterestCamera camera_{};
    std::unordered_map<std::uint64_t, WorldInterestSource> sources_;
};

}
