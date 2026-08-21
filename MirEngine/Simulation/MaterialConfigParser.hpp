#pragma once

#include "MaterialProperties.hpp"

#include <algorithm>
#include <string>
#include <string_view>

namespace mir
{

class MaterialConfigParser
{
public:
    [[nodiscard]] static bool parse(
        std::string_view text,
        MaterialProperties& material) noexcept
    {
        bool changed = false;
        std::size_t position = 0;

        while (position < text.size())
        {
            const auto end = text.find('\n', position);
            auto line = text.substr(
                position,
                end == std::string_view::npos ? text.size() - position : end - position);

            line = trim(line);
            if (!line.empty() && line.front() != '#')
            {
                const auto separator = line.find('=');
                if (separator != std::string_view::npos)
                {
                    const auto key = trim(line.substr(0, separator));
                    const auto value = trim(line.substr(separator + 1));
                    changed |= apply(key, value, material);
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
        while (!value.empty() &&
               (value.front() == ' ' || value.front() == '\t' || value.front() == '\r'))
            value.remove_prefix(1);

        while (!value.empty() &&
               (value.back() == ' ' || value.back() == '\t' || value.back() == '\r'))
            value.remove_suffix(1);

        return value;
    }

    static Scalar number(std::string_view value, Scalar fallback) noexcept
    {
        try
        {
            return std::stod(std::string(value));
        }
        catch (...)
        {
            return fallback;
        }
    }

    static bool apply(
        std::string_view key,
        std::string_view value,
        MaterialProperties& material) noexcept
    {
        if (key == "name")
        {
            material.name = std::string(value);
            return true;
        }

        if (key == "density")
        {
            material.density = std::max(0.0, number(value, material.density));
            return true;
        }

        if (key == "viscosity")
        {
            material.viscosity = std::max(0.0, number(value, material.viscosity));
            return true;
        }

        if (key == "specificHeat")
        {
            material.specificHeat = std::max(0.0, number(value, material.specificHeat));
            return true;
        }

        if (key == "thermalConductivity")
        {
            material.thermalConductivity =
                std::max(0.0, number(value, material.thermalConductivity));
            return true;
        }

        if (key == "referenceTemperature")
        {
            material.referenceTemperature = number(value, material.referenceTemperature);
            return true;
        }

        constexpr std::string_view prefix = "composition.";
        if (key.starts_with(prefix))
        {
            const auto component = key.substr(prefix.size());
            if (!component.empty())
            {
                const std::string name{component};
                const Scalar fraction = std::max(0.0, number(value, 0.0));
                bool found = false;
                for (auto& entry : material.composition)
                {
                    if (entry.name == name)
                    {
                        entry.fraction = fraction;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    material.composition.push_back({name, fraction});
                material.normalizeComposition();
                return true;
            }
        }

        return false;
    }
};

} // namespace mir
