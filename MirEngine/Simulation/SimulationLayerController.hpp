#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace mir
{

enum class SimulationLayer : std::uint8_t
{
    Geometry,
    Material,
    Flow,
    Pressure,
    Temperature,
    Chemistry,
    Aerodynamics,
    Acoustics,
    Particles,
    Vectors
};

struct SimulationLayerState
{
    bool enabled{true};
    bool isolated{false};
    bool transparent{false};
    float opacity{1.0f};
    int order{0};
};

class SimulationLayerController
{
public:
    SimulationLayerController()
    {
        configureDefaults();
    }

    void configureDefaults() noexcept
    {
        layers_.clear();
        set(SimulationLayer::Geometry, true, false, false, 1.0f, 0);
        set(SimulationLayer::Material, true, false, false, 1.0f, 1);
        set(SimulationLayer::Flow, false, false, true, 0.8f, 2);
        set(SimulationLayer::Pressure, false, false, true, 0.8f, 3);
        set(SimulationLayer::Temperature, false, false, true, 0.8f, 4);
        set(SimulationLayer::Chemistry, false, false, true, 0.8f, 5);
        set(SimulationLayer::Aerodynamics, false, false, true, 0.8f, 6);
        set(SimulationLayer::Acoustics, false, false, true, 0.8f, 7);
        set(SimulationLayer::Particles, true, false, false, 1.0f, 8);
        set(SimulationLayer::Vectors, false, false, true, 0.9f, 9);
    }

    void set(
        SimulationLayer layer,
        bool enabled,
        bool isolated,
        bool transparent,
        float opacity,
        int order) noexcept
    {
        SimulationLayerState state;
        state.enabled = enabled;
        state.isolated = isolated;
        state.transparent = transparent;
        state.opacity = std::clamp(opacity, 0.0f, 1.0f);
        state.order = order;
        layers_[layer] = state;
    }

    void enable(SimulationLayer layer, bool value = true) noexcept
    {
        layers_[layer].enabled = value;
    }

    void isolate(SimulationLayer layer) noexcept
    {
        for (auto& [other, state] : layers_)
        {
            state.isolated = other == layer;
        }
    }

    void clearIsolation() noexcept
    {
        for (auto& [layer, state] : layers_)
        {
            state.isolated = false;
        }
    }

    void setOpacity(SimulationLayer layer, float opacity) noexcept
    {
        layers_[layer].opacity = std::clamp(opacity, 0.0f, 1.0f);
    }

    [[nodiscard]] bool visible(SimulationLayer layer) const noexcept
    {
        const auto it = layers_.find(layer);
        if (it == layers_.end())
            return false;

        bool anyIsolated = false;
        for (const auto& [_, state] : layers_)
        {
            if (state.isolated)
            {
                anyIsolated = true;
                break;
            }
        }

        return it->second.enabled && (!anyIsolated || it->second.isolated);
    }

    [[nodiscard]] const SimulationLayerState& state(SimulationLayer layer) const noexcept
    {
        return layers_.at(layer);
    }

private:
    std::unordered_map<SimulationLayer, SimulationLayerState> layers_;
};

}
