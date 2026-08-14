#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace mir
{

struct RGB
{
    float r{0.0F};
    float g{0.0F};
    float b{0.0F};
};

struct MaterialOpticalProperties
{
    // Base reflectance in linear RGB. This is not a texture: it describes
    // how the material responds to incident light.
    RGB reflectance{0.7F, 0.7F, 0.7F};

    float metallic{0.0F};
    float roughness{0.5F};
    float specular{0.5F};
    float transmission{0.0F};
    float indexOfRefraction{1.5F};
    float absorption{0.0F};

    [[nodiscard]] RGB reflectedLight(
        RGB incident,
        float normalDotLight,
        float normalDotView) const noexcept
    {
        const float ndl = std::clamp(normalDotLight, 0.0F, 1.0F);
        const float ndv = std::clamp(normalDotView, 0.0F, 1.0F);
        const float smooth = 1.0F - std::clamp(roughness, 0.0F, 1.0F);
        const float fresnel = specular + (1.0F - specular) *
            std::pow(1.0F - ndv, 5.0F);
        const float energy = ndl * (0.15F + 0.85F * smooth) *
            (0.25F + 0.75F * fresnel);

        return {
            incident.r * reflectance.r * energy,
            incident.g * reflectance.g * energy,
            incident.b * reflectance.b * energy};
    }
};

} // namespace mir
