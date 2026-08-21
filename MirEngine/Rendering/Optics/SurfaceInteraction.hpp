#pragma once

#include "OpticalMaterial.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace mir
{

struct OpticalLight
{
    std::array<float, 3> intensity{1.0F, 1.0F, 1.0F};
    float wavelengthNm{550.0F};
};

struct OpticalSurfacePoint
{
    std::array<float, 3> normal{0.0F, 0.0F, 1.0F};
    std::array<float, 3> viewDirection{0.0F, 0.0F, 1.0F};
    std::array<float, 3> lightDirection{0.0F, 0.0F, 1.0F};
};

struct SurfaceOpticalResponse
{
    std::array<float, 3> reflected{0.0F, 0.0F, 0.0F};
    std::array<float, 3> transmitted{0.0F, 0.0F, 0.0F};
    float reflectedEnergy{0.0F};
    float transmittedEnergy{0.0F};
};

class SurfaceInteraction
{
public:
    [[nodiscard]] static SurfaceOpticalResponse evaluate(
        const OpticalMaterial& material,
        const OpticalLight& light,
        const OpticalSurfacePoint& surface) noexcept
    {
        const float ndotl = std::clamp(dot(surface.normal, surface.lightDirection), 0.0F, 1.0F);
        const float roughness = std::clamp(material.roughness, 0.0F, 1.0F);
        const float metallic = std::clamp(material.metallic, 0.0F, 1.0F);

        SurfaceOpticalResponse result{};

        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            const float reflectance = std::clamp(material.reflectance(channel), 0.0F, 1.0F);
            const float absorption = std::clamp(material.absorptionAt(channel), 0.0F, 1.0F);
            const float transmission = std::clamp(material.transmissionAt(channel), 0.0F, 1.0F);

            const float f0 = metallic * reflectance +
                (1.0F - metallic) * dielectricReflectance(material.indexOfRefraction);
            const float viewFactor = std::clamp(
                dot(surface.normal, surface.viewDirection), 0.0F, 1.0F);
            const float fresnel = f0 + (1.0F - f0) * std::pow(1.0F - viewFactor, 5.0F);
            const float diffuse = (1.0F - fresnel) * (1.0F - roughness);

            result.reflected[channel] =
                light.intensity[channel] * ndotl * std::clamp(diffuse * reflectance + fresnel, 0.0F, 1.0F);

            result.transmitted[channel] =
                light.intensity[channel] * ndotl * transmission * (1.0F - absorption);
        }

        result.reflectedEnergy = average(result.reflected);
        result.transmittedEnergy = average(result.transmitted);
        return result;
    }

private:
    [[nodiscard]] static float dot(
        const std::array<float, 3>& a,
        const std::array<float, 3>& b) noexcept
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

    [[nodiscard]] static float dielectricReflectance(float ior) noexcept
    {
        const float safeIor = std::max(1.0F, ior);
        const float r = (safeIor - 1.0F) / (safeIor + 1.0F);
        return r * r;
    }

    [[nodiscard]] static float average(const std::array<float, 3>& value) noexcept
    {
        return (value[0] + value[1] + value[2]) / 3.0F;
    }
};

}
