#pragma once

#include "MaterialInterface.hpp"

#include <algorithm>
#include <string>
#include <string_view>

namespace mir
{

class MaterialInterfaceConfigParser
{
public:
    [[nodiscard]] static bool parse(
        std::string_view text,
        MaterialInterface& interfaceValue) noexcept
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
                    changed |= apply(key, value, interfaceValue);
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

    static Scalar number(std::string_view value, Scalar fallback) noexcept
    {
        try { return std::stod(std::string(value)); }
        catch (...) { return fallback; }
    }

    static std::uint64_t integer(std::string_view value, std::uint64_t fallback) noexcept
    {
        try { return std::stoull(std::string(value)); }
        catch (...) { return fallback; }
    }

    static bool boolean(std::string_view value, bool fallback) noexcept
    {
        if (value == "true" || value == "1" || value == "on") return true;
        if (value == "false" || value == "0" || value == "off") return false;
        return fallback;
    }

    static bool apply(
        std::string_view key,
        std::string_view value,
        MaterialInterface& interfaceValue) noexcept
    {
        if (key == "firstRegionId")
        {
            interfaceValue.firstRegionId = integer(value, interfaceValue.firstRegionId);
            return true;
        }
        if (key == "secondRegionId")
        {
            interfaceValue.secondRegionId = integer(value, interfaceValue.secondRegionId);
            return true;
        }
        if (key == "coefficient")
        {
            interfaceValue.coefficient = std::max(0.0, number(value, interfaceValue.coefficient));
            return true;
        }
        if (key == "enabled")
        {
            interfaceValue.enabled = boolean(value, interfaceValue.enabled);
            return true;
        }
        if (key == "process")
        {
            if (value == "thermal" || value == "thermalTransfer")
                interfaceValue.process = MaterialInterfaceProcess::ThermalTransfer;
            else if (value == "diffusion")
                interfaceValue.process = MaterialInterfaceProcess::Diffusion;
            else if (value == "contact")
                interfaceValue.process = MaterialInterfaceProcess::Contact;
            else if (value == "chemical" || value == "chemicalReaction")
                interfaceValue.process = MaterialInterfaceProcess::ChemicalReaction;
            else
                return false;
            return true;
        }
        return false;
    }
};

}
