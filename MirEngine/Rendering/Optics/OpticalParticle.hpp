#pragma once

#include "SpectralLight.hpp"

#include <algorithm>
#include <cmath>

namespace mir
{

struct OpticalParticle
{
    WorldPosition position{};
    float radius{0.001F};
    float density{1.0F};
    float scattering{0.0F};
    float absorption{0.0F};
    float emission{0.0F};

    [[nodiscard]] float interact(const SpectralLight& light, float wavelengthNm) const noexcept
    {
        const float incident = std::max(0.0F, light.powerAt(wavelengthNm));
        const float attenuation = std::clamp(absorption, 0.0F, 1.0F);
        const float scatter = std::clamp(scattering, 0.0F, 1.0F);
        const float emitted = std::max(0.0F, emission);

        return incident * (1.0F - attenuation) * scatter + emitted;
    }
};

}
