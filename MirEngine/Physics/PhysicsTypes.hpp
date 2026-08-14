#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Math/Vector/Vector3.hpp"
#include "../World/WorldObject.hpp"

namespace mir
{

struct PhysicsState
{
    Vector3 velocity{};
    Vector3 acceleration{};
    Vector3 force{};
    Scalar mass{1.0};
    Scalar gravityScale{1.0};
    bool enabled{true};
    bool dynamic{true};
};

struct CollisionEvent
{
    WorldObject::Id first{0};
    WorldObject::Id second{0};
    Vector3 point{};
    Vector3 normal{};
    Scalar impulse{0.0};
};

} // namespace mir
