#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace mir
{

enum class ChemicalPhase : std::uint8_t
{
    Solid,
    Liquid,
    Gas,
    Plasma,
    Unknown
};

struct ChemicalComponent
{
    std::uint32_t id{0};
    std::string name;
    float massFraction{0.0F};
    float molarFraction{0.0F};
};

class MaterialComposition
{
public:
    void setPhase(ChemicalPhase phase) noexcept
    {
        phase_ = phase;
    }

    void setTemperatureK(float temperatureK) noexcept
    {
        temperatureK_ = std::max(0.0F, temperatureK);
    }

    void setPressurePa(float pressurePa) noexcept
    {
        pressurePa_ = std::max(0.0F, pressurePa);
    }

    void addComponent(ChemicalComponent component)
    {
        component.massFraction = std::max(0.0F, component.massFraction);
        component.molarFraction = std::max(0.0F, component.molarFraction);
        components_.push_back(std::move(component));
    }

    [[nodiscard]] bool adjustMolarFraction(
        std::uint32_t componentId,
        float delta) noexcept
    {
        for (auto& component : components_)
        {
            if (component.id != componentId)
                continue;

            component.molarFraction = std::max(
                0.0F,
                component.molarFraction + delta);
            return true;
        }
        return false;
    }

    [[nodiscard]] ChemicalPhase phase() const noexcept
    {
        return phase_;
    }

    [[nodiscard]] float temperatureK() const noexcept
    {
        return temperatureK_;
    }

    [[nodiscard]] float pressurePa() const noexcept
    {
        return pressurePa_;
    }

    [[nodiscard]] const std::vector<ChemicalComponent>& components() const noexcept
    {
        return components_;
    }

    [[nodiscard]] float totalMassFraction() const noexcept
    {
        float total = 0.0F;
        for (const auto& component : components_)
            total += component.massFraction;
        return total;
    }

    [[nodiscard]] float totalMolarFraction() const noexcept
    {
        float total = 0.0F;
        for (const auto& component : components_)
            total += component.molarFraction;
        return total;
    }

private:
    ChemicalPhase phase_{ChemicalPhase::Unknown};
    float temperatureK_{293.15F};
    float pressurePa_{101325.0F};
    std::vector<ChemicalComponent> components_;
};

}