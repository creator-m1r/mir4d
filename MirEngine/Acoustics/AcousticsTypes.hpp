#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Math/Vector/Vector.hpp"
#include "../World/WorldObject.hpp"

namespace mir
{

struct AcousticEvent
{
    WorldObject::Id source{0};
    Vector3 position{};
    Scalar intensity{0.0};
    Scalar frequency{440.0};
    Scalar duration{0.1};
};

struct AcousticSettings
{
    bool enabled{true};
    Scalar masterVolume{1.0};
    Scalar speedOfSound{343.0};
    Scalar maxDistance{1000.0};
};

} // namespace mir
