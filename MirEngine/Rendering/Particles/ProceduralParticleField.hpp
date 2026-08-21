#pragma once

#include "WorldParticle.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace mir
{

enum class ProceduralParticleMaterial : std::uint8_t
{
    Water,
    Air,
    Soil,
    Metal,
    Smoke
};

struct ProceduralParticleFieldSettings
{
    std::uint64_t firstParticleId{1};
    std::size_t particleCount{0};
    float spacing{0.05F};
    float jitter{0.0F};
    float originX{0.0F};
    float originY{0.0F};
    float originZ{0.0F};
};

class ProceduralParticleField
{
public:
    [[nodiscard]] static std::vector<WorldParticle> generate(
        const ProceduralParticleFieldSettings& settings,
        ProceduralParticleMaterial material)
    {
        std::vector<WorldParticle> particles;
        particles.reserve(settings.particleCount);

        if (settings.particleCount == 0 || settings.spacing <= 0.0F)
            return particles;

        const auto side = static_cast<std::size_t>(std::ceil(
            std::cbrt(static_cast<double>(settings.particleCount))));
        const auto safeSide = std::max<std::size_t>(side, 1);

        for (std::size_t index = 0;
             index < settings.particleCount;
             ++index)
        {
            const std::size_t x = index % safeSide;
            const std::size_t y = (index / safeSide) % safeSide;
            const std::size_t z = index / (safeSide * safeSide);

            WorldParticle particle{};
            particle.id = settings.firstParticleId + index;
            particle.x = settings.originX + static_cast<float>(x) * settings.spacing;
            particle.y = settings.originY + static_cast<float>(y) * settings.spacing;
            particle.z = settings.originZ + static_cast<float>(z) * settings.spacing;
            particle.radius = settings.spacing * 0.45F;
            particle.optical = opticalProperties(material);
            particles.push_back(particle);
        }

        return particles;
    }

private:
    [[nodiscard]] static MaterialOpticalProperties opticalProperties(
        ProceduralParticleMaterial material) noexcept
    {
        MaterialOpticalProperties result{};

        switch (material)
        {
        case ProceduralParticleMaterial::Water:
            result.reflectance = {0.72F, 0.82F, 0.92F};
            result.metallic = 0.0F;
            result.roughness = 0.08F;
            result.specular = 0.5F;
            result.transmission = 0.92F;
            result.indexOfRefraction = 1.333F;
            result.absorption = 0.02F;
            break;

        case ProceduralParticleMaterial::Air:
            result.reflectance = {0.98F, 0.99F, 1.0F};
            result.roughness = 1.0F;
            result.specular = 0.02F;
            result.transmission = 1.0F;
            result.indexOfRefraction = 1.000293F;
            result.absorption = 0.0F;
            break;

        case ProceduralParticleMaterial::Soil:
            result.reflectance = {0.22F, 0.14F, 0.08F};
            result.roughness = 0.92F;
            result.specular = 0.05F;
            break;

        case ProceduralParticleMaterial::Metal:
            result.reflectance = {0.82F, 0.84F, 0.86F};
            result.metallic = 1.0F;
            result.roughness = 0.28F;
            result.specular = 0.9F;
            break;

        case ProceduralParticleMaterial::Smoke:
            result.reflectance = {0.55F, 0.55F, 0.55F};
            result.roughness = 1.0F;
            result.specular = 0.0F;
            result.transmission = 0.65F;
            result.absorption = 0.2F;
            break;
        }

        return result;
    }
};

}
