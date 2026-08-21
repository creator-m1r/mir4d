#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace mir
{

struct SpectralSample
{
    float wavelengthNm{550.0F};
    float value{0.0F};
};

struct OpticalMaterial
{
    std::array<float, 3> baseReflectance{0.5F, 0.5F, 0.5F};
    std::array<float, 3> absorption{0.0F, 0.0F, 0.0F};
    std::array<float, 3> transmission{0.0F, 0.0F, 0.0F};

    float roughness{0.5F};
    float metallic{0.0F};
    float indexOfRefraction{1.0F};
    float densityKgM3{1000.0F};

    bool transparent{false};

    [[nodiscard]] float reflectance(std::size_t channel) const noexcept
    {
        return baseReflectance[std::min<std::size_t>(channel, 2)];
    }

    [[nodiscard]] float transmissionAt(std::size_t channel) const noexcept
    {
        return transmission[std::min<std::size_t>(channel, 2)];
    }

    [[nodiscard]] float absorptionAt(std::size_t channel) const noexcept
    {
        return absorption[std::min<std::size_t>(channel, 2)];
    }
};

} // namespace mir
