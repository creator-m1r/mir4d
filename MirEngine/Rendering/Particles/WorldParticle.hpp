#pragma once

#include "../Material/MaterialOpticalProperties.hpp"

#include <cstdint>

namespace mir
{

struct WorldParticle
{
    std::uint64_t id{0};
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};
    float radius{0.01F};

    // Optical response is generated from physical/material properties.
    MaterialOpticalProperties optical{};

    bool visible{true};
};

} // namespace mir
