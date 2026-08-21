#pragma once

#include "OpticalParticle.hpp"

#include <cstddef>
#include <vector>

namespace mir
{

class OpticalWorld
{
public:
    void setLight(const SpectralLight& light) noexcept
    {
        light_ = light;
    }

    void addParticle(const OpticalParticle& particle)
    {
        particles_.push_back(particle);
    }

    void clearParticles() noexcept
    {
        particles_.clear();
    }

    [[nodiscard]] float sample(float wavelengthNm) const noexcept
    {
        float result = 0.0F;
        for (const auto& particle : particles_)
            result += particle.interact(light_, wavelengthNm);
        return result;
    }

    [[nodiscard]] std::size_t particleCount() const noexcept
    {
        return particles_.size();
    }

    [[nodiscard]] const SpectralLight& light() const noexcept
    {
        return light_;
    }

private:
    SpectralLight light_{};
    std::vector<OpticalParticle> particles_;
};

}
