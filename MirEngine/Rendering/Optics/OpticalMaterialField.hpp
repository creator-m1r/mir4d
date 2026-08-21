#pragma once

#include "SpectralLight.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace mir
{

struct OpticalMaterialSample
{
    float wavelengthNm{550.0F};
    float absorption{0.0F};
    float scattering{0.0F};
    float transmission{1.0F};
    float emission{0.0F};
    float refractiveIndex{1.0F};
};

class OpticalMaterialField
{
public:
    explicit OpticalMaterialField(std::size_t sampleCount = 7)
        : samples_(sampleCount)
    {
        initialiseWavelengths();
    }

    void setSample(std::size_t index, OpticalMaterialSample sample) noexcept
    {
        if (index >= samples_.size())
            return;

        sample.absorption = std::clamp(sample.absorption, 0.0F, 1.0F);
        sample.scattering = std::clamp(sample.scattering, 0.0F, 1.0F);
        sample.transmission = std::clamp(sample.transmission, 0.0F, 1.0F);
        sample.refractiveIndex = std::max(1.0F, sample.refractiveIndex);
        samples_[index] = sample;
    }

    [[nodiscard]] float response(
        const SpectralLight& light,
        float wavelengthNm) const noexcept
    {
        if (samples_.empty())
            return 0.0F;

        const auto sample = interpolate(wavelengthNm);
        const float incident = std::max(0.0F, light.powerAt(wavelengthNm));
        const float absorbed = incident * sample.absorption;
        const float scattered = incident * sample.scattering;
        const float transmitted = incident * sample.transmission;

        return std::max(0.0F, transmitted + scattered - absorbed + sample.emission);
    }

    [[nodiscard]] OpticalMaterialSample sampleAt(float wavelengthNm) const noexcept
    {
        return interpolate(wavelengthNm);
    }

    [[nodiscard]] std::size_t sampleCount() const noexcept
    {
        return samples_.size();
    }

private:
    void initialiseWavelengths() noexcept
    {
        if (samples_.empty())
            return;

        if (samples_.size() == 1)
        {
            samples_[0].wavelengthNm = 550.0F;
            return;
        }

        constexpr float first = 400.0F;
        constexpr float last = 700.0F;
        const float step = (last - first) / static_cast<float>(samples_.size() - 1);

        for (std::size_t i = 0; i < samples_.size(); ++i)
            samples_[i].wavelengthNm = first + step * static_cast<float>(i);
    }

    [[nodiscard]] OpticalMaterialSample interpolate(float wavelengthNm) const noexcept
    {
        if (wavelengthNm <= samples_.front().wavelengthNm)
            return samples_.front();
        if (wavelengthNm >= samples_.back().wavelengthNm)
            return samples_.back();

        for (std::size_t i = 1; i < samples_.size(); ++i)
        {
            const auto& a = samples_[i - 1];
            const auto& b = samples_[i];
            if (wavelengthNm <= b.wavelengthNm)
            {
                const float span = b.wavelengthNm - a.wavelengthNm;
                const float t = span > 0.0F
                    ? (wavelengthNm - a.wavelengthNm) / span
                    : 0.0F;

                return {
                    wavelengthNm,
                    a.absorption + (b.absorption - a.absorption) * t,
                    a.scattering + (b.scattering - a.scattering) * t,
                    a.transmission + (b.transmission - a.transmission) * t,
                    a.emission + (b.emission - a.emission) * t,
                    a.refractiveIndex + (b.refractiveIndex - a.refractiveIndex) * t};
            }
        }

        return samples_.back();
    }

    std::vector<OpticalMaterialSample> samples_;
};

}
