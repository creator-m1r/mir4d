#pragma once

#include "PhysicsTypes.hpp"
#include "../World/World.hpp"

#include <unordered_map>

namespace mir
{

class PhysicsWorld
{
public:
    void attach(WorldObject::Id id, const PhysicsState& state = {})
    {
        states_[id] = state;
    }

    void detach(WorldObject::Id id) noexcept
    {
        states_.erase(id);
    }

    [[nodiscard]] PhysicsState* state(WorldObject::Id id) noexcept
    {
        const auto it = states_.find(id);
        return it == states_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const PhysicsState* state(WorldObject::Id id) const noexcept
    {
        const auto it = states_.find(id);
        return it == states_.end() ? nullptr : &it->second;
    }

    void step(World& world, Scalar deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        for (auto& [id, physics] : states_)
        {
            if (!physics.enabled || !physics.dynamic || physics.mass <= 0.0)
                continue;

            auto object = world.find(id);
            if (!object)
                continue;

            physics.acceleration = physics.force / physics.mass;
            physics.velocity += physics.acceleration * deltaSeconds;
            object->state().position += physics.velocity * deltaSeconds;
        }
    }

private:
    std::unordered_map<WorldObject::Id, PhysicsState> states_{};
};

}
