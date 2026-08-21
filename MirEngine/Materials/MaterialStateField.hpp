#pragma once

#include "../Rendering/Optics/OpticalMaterialField.hpp"

#include <algorithm>
#include <cstdint>

namespace mir
{

struct MaterialState
{
    float temperatureK{293.15F};
    float pressurePa{101325.0F};
    float densityKgM3{1.0F};
    float viscosityPaS{0.001F};
    float specificHeatJKgK{4181.0F};
    float thermalConductivityWmK{0.6F};
    float electricalConductivitySm{0.0F};
    float phaseFraction{1.0F};
    std::uint32_t chemicalCompositionId{0};
};

class MaterialStateField
{
public:
    MaterialStateField() = default;

    explicit MaterialStateField(MaterialState state) noexcept
        : state_(state)
    {
    }

    void setState(MaterialState state) noexcept
    {
        state.temperatureK = std::max(0.0F, state.temperatureK);
        state.pressurePa = std::max(0.0F, state.pressurePa);
        state.densityKgM3 = std::max(0.0F, state.densityKgM3);
        state.viscosityPaS = std::max(0.0F, state.viscosityPaS);
        state.specificHeatJKgK = std::max(0.0F, state.specificHeatJKgK);
        state.thermalConductivityWmK = std::max(0.0F, state.thermalConductivityWmK);
        state.electricalConductivitySm = std::max(0.0F, state.electricalConductivitySm);
        state.phaseFraction = std::clamp(state.phaseFraction, 0.0F, 1.0F);
        state_ = state;
    }

    [[nodiscard]] const MaterialState& state() const noexcept
    {
        return state_;
    }

    [[nodiscard]] MaterialState& state() noexcept
    {
        return state_;
    }

    void setOpticalField(const OpticalMaterialField& field)
    {
        opticalField_ = field;
    }

    [[nodiscard]] const OpticalMaterialField& opticalField() const noexcept
    {
        return opticalField_;
    }

    [[nodiscard]] OpticalMaterialField& opticalField() noexcept
    {
        return opticalField_;
    }

private:
    MaterialState state_{};
    OpticalMaterialField opticalField_{};
};

}
