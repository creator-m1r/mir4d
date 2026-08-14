#pragma once

#include "WorldParticle.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace mir
{

enum class ParticleLODLevel : unsigned char
{
    Far = 0,
    Medium = 1,
    Near = 2,
    Detailed = 3
};

struct ParticleLODSettings
{
    float mediumDistance{40.0F};
    float nearDistance{12.0F};
    float detailedDistance{3.0F};

    std::size_t farBudget{512};
    std::size_t mediumBudget{4096};
    std::size_t nearBudget{16384};
    std::size_t detailedBudget{65536};
};

struct ParticleLODSelection
{
    ParticleLODLevel level{ParticleLODLevel::Far};
    std::size_t visibleCount{0};
    std::size_t sourceCount{0};
};

class ParticleLODSystem
{
public:
    [[nodiscard]] ParticleLODSelection select(
        const std::vector<WorldParticle>& particles,
        float cameraX,
        float cameraY,
        float cameraZ,
        const ParticleLODSettings& settings = {}) const
    {
        ParticleLODSelection result{};
        result.sourceCount = particles.size();

        const float nearestDistanceSquared = nearestDistanceSquaredToCamera(
            particles, cameraX, cameraY, cameraZ);
        const float nearestDistance = std::sqrt(nearestDistanceSquared);

        if (nearestDistance <= settings.detailedDistance)
        {
            result.level = ParticleLODLevel::Detailed;
            result.visibleCount = std::min(settings.detailedBudget, particles.size());
        }
        else if (nearestDistance <= settings.nearDistance)
        {
            result.level = ParticleLODLevel::Near;
            result.visibleCount = std::min(settings.nearBudget, particles.size());
        }
        else if (nearestDistance <= settings.mediumDistance)
        {
            result.level = ParticleLODLevel::Medium;
            result.visibleCount = std::min(settings.mediumBudget, particles.size());
        }
        else
        {
            result.level = ParticleLODLevel::Far;
            result.visibleCount = std::min(settings.farBudget, particles.size());
        }

        return result;
    }

private:
    [[nodiscard]] static float nearestDistanceSquaredToCamera(
        const std::vector<WorldParticle>& particles,
        float cameraX,
        float cameraY,
        float cameraZ) noexcept
    {
        if (particles.empty())
            return 1.0e30F;

        float minimum = 1.0e30F;
        for (const auto& particle : particles)
        {
            if (!particle.visible)
                continue;

            const float dx = particle.x - cameraX;
            const float dy = particle.y - cameraY;
            const float dz = particle.z - cameraZ;
            minimum = std::min(minimum, dx * dx + dy * dy + dz * dz);
        }
        return minimum;
    }
};

} // namespace mir
