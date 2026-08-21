#pragma once

#include "SimulationLayerController.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

namespace mir
{

struct SimulationLayerConfig
{
    bool enabled{true};
    bool isolated{false};
    bool transparent{false};
    float opacity{1.0f};
    int order{0};
};

class SimulationLayerConfigParser
{
public:
    static bool apply(std::string_view text, SimulationLayerController& controller) noexcept
    {
        bool changed = false;
        std::size_t position = 0;

        while (position < text.size())
        {
            const std::size_t end = text.find('\n', position);
            const std::string_view line = text.substr(
                position,
                end == std::string_view::npos ? text.size() - position : end - position);

            if (!line.empty() && line.front() != '#')
            {
                const std::size_t separator = line.find('=');
                if (separator != std::string_view::npos)
                {
                    const std::string_view key = trim(line.substr(0, separator));
                    const std::string_view value = trim(line.substr(separator + 1));
                    changed |= applyValue(key, value, controller);
                }
            }

            if (end == std::string_view::npos)
                break;
            position = end + 1;
        }

        return changed;
    }

private:
    static std::string_view trim(std::string_view value) noexcept
    {
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t' || value.front() == '\r'))
            value.remove_prefix(1);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r'))
            value.remove_suffix(1);
        return value;
    }

    static bool boolean(std::string_view value, bool fallback) noexcept
    {
        if (value == "true" || value == "1" || value == "on") return true;
        if (value == "false" || value == "0" || value == "off") return false;
        return fallback;
    }

    static float number(std::string_view value, float fallback) noexcept
    {
        try
        {
            return std::stof(std::string(value));
        }
        catch (...)
        {
            return fallback;
        }
    }

    static bool applyValue(
        std::string_view key,
        std::string_view value,
        SimulationLayerController& controller) noexcept
    {
        struct Entry
        {
            std::string_view name;
            SimulationLayer layer;
        };

        constexpr Entry entries[] = {
            {"geometry", SimulationLayer::Geometry},
            {"material", SimulationLayer::Material},
            {"flow", SimulationLayer::Flow},
            {"pressure", SimulationLayer::Pressure},
            {"temperature", SimulationLayer::Temperature},
            {"chemistry", SimulationLayer::Chemistry},
            {"aerodynamics", SimulationLayer::Aerodynamics},
            {"acoustics", SimulationLayer::Acoustics},
            {"particles", SimulationLayer::Particles},
            {"vectors", SimulationLayer::Vectors}
        };

        for (const auto& entry : entries)
        {
            const std::string prefix(entry.name);
            if (key == prefix + ".enabled")
            {
                controller.enable(entry.layer, boolean(value, controller.state(entry.layer).enabled));
                return true;
            }
            if (key == prefix + ".opacity")
            {
                controller.setOpacity(entry.layer, number(value, controller.state(entry.layer).opacity));
                return true;
            }
        }

        return false;
    }
};

} // namespace mir
