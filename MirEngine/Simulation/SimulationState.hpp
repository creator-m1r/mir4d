#pragma once

#include "MaterialInteractionGraph.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

namespace mir
{

struct SimulationVector3
{
    Scalar x{0.0};
    Scalar y{0.0};
    Scalar z{0.0};
};

struct SimulationState
{
    Scalar time{0.0};
    Scalar deltaTime{0.0};

    Scalar temperature{293.15};
    Scalar pressure{101325.0};
    Scalar flowRate{0.0};
    Scalar density{0.0};
    Scalar viscosity{0.0};

    SimulationVector3 velocity{};
    SimulationVector3 acceleration{};

    Scalar acousticLevel{0.0};
    Scalar aerodynamicPressure{0.0};
    Scalar aerodynamicDrag{0.0};

    Scalar stress{0.0};
    Scalar strain{0.0};
    Scalar displacement{0.0};

    Scalar youngModulus{2.0e11};
    Scalar thermalExpansion{1.2e-5};
    Scalar poissonRatio{0.3};
    Scalar specificHeat{4181.0};
    Scalar thermalConductivity{0.6};
    Scalar referenceDensity{1000.0};
    Scalar heatFlux{0.0};
    Scalar pressureLoad{0.0};
    bool fixed{false};

    std::unordered_map<std::string, Scalar> composition;

    bool running{false};
    bool paused{false};

    void reset() noexcept
    {
        time = 0.0;
        deltaTime = 0.0;
        temperature = 293.15;
        pressure = 101325.0;
        flowRate = 0.0;
        density = 0.0;
        viscosity = 0.0;
        velocity = {};
        acceleration = {};
        acousticLevel = 0.0;
        aerodynamicPressure = 0.0;
        aerodynamicDrag = 0.0;
        composition.clear();
        running = false;
        paused = false;
    }
};

class SimulationStateStore
{
public:
    [[nodiscard]] SimulationState& state(std::uint64_t objectId) noexcept
    {
        return states_[objectId];
    }

    [[nodiscard]] const SimulationState* find(std::uint64_t objectId) const noexcept
    {
        const auto it = states_.find(objectId);
        return it == states_.end() ? nullptr : &it->second;
    }

    void remove(std::uint64_t objectId) noexcept
    {
        states_.erase(objectId);
    }

    void clear() noexcept
    {
        states_.clear();
    }

    void start(std::uint64_t objectId) noexcept
    {
        auto& value = states_[objectId];
        value.running = true;
        value.paused = false;
    }

    void pause(std::uint64_t objectId) noexcept
    {
        if (auto* value = mutableFind(objectId))
            value->paused = true;
    }

    void resume(std::uint64_t objectId) noexcept
    {
        if (auto* value = mutableFind(objectId))
        {
            value->running = true;
            value->paused = false;
        }
    }

    void stop(std::uint64_t objectId) noexcept
    {
        if (auto* value = mutableFind(objectId))
        {
            value->running = false;
            value->paused = false;
        }
    }

    void advance(std::uint64_t objectId, Scalar deltaTime) noexcept
    {
        auto* value = mutableFind(objectId);
        if (!value || !value->running || value->paused)
            return;

        value->deltaTime = std::max(0.0, deltaTime);
        value->time += value->deltaTime;
    }

    template <typename F>
    void forEach(F&& fn) noexcept
    {
        for (auto& [id, state] : states_)
            fn(id, state);
    }

    template <typename F>
    void forEach(F&& fn) const noexcept
    {
        for (const auto& [id, state] : states_)
            fn(id, state);
    }

private:
    [[nodiscard]] SimulationState* mutableFind(std::uint64_t objectId) noexcept
    {
        const auto it = states_.find(objectId);
        return it == states_.end() ? nullptr : &it->second;
    }

    std::unordered_map<std::uint64_t, SimulationState> states_;
};

} // namespace mir
