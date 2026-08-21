#pragma once

#include <algorithm>
#include <array>
#include <cmath>

namespace mir
{

struct SpectralSample
{
    float wavelengthNm{550.0F};
    float power{1.0F};
};

struct SpectralLight
{
    std::array<SpectralSample, 7> samples{
        SpectralSample{400.0F, 0.0F},
        SpectralSample{450.0F, 0.0F},
        SpectralSample{500.0F, 0.0F},
        SpectralSample{550.0F, 1.0F},
        SpectralSample{600.0F, 0.0F},
        SpectralSample{650.0F, 0.0F},
        SpectralSample{700.0F, 0.0F}};

    [[nodiscard]] float powerAt(float wavelengthNm) const noexcept
    {
        if (wavelengthNm <= samples.front().wavelengthNm)
            return samples.front().power;
        if (wavelengthNm >= samples.back().wavelengthNm)
            return samples.back().power;

        for (std::size_t i = 1; i < samples.size(); ++i)
        {
            const auto& a = samples[i - 1];
            const auto& b = samples[i];
            if (wavelengthNm <= b.wavelengthNm)
            {
                const float span = b.wavelengthNm - a.wavelengthNm;
                const float t = span > 0.0F
                    ? (wavelengthNm - a.wavelengthNm) / span
                    : 0.0F;
                return std::max(0.0F, a.power + (b.power - a.power) * t);
            }
        }
        return 0.0F;
    }
};

} // namespace mir
