#pragma once

#include "ProcessMaterial.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace mir
{

enum class SimulationFieldType
{
    FlowRate,
    Pressure,
    Temperature,
    ChemicalConcentration,
    AirVelocity
};

struct SimulationFieldSample
{
    std::uint64_t objectId{0};
    SimulationFieldType type{SimulationFieldType::Temperature};
    Scalar value{0.0};
    Scalar minimum{0.0};
    Scalar maximum{1.0};
};

class SimulationFieldStore
{
public:
    void clear() noexcept
    {
        samples_.clear();
    }

    void set(SimulationFieldSample sample) noexcept
    {
        sample.minimum = std::min(sample.minimum, sample.maximum);
        sample.maximum = std::max(sample.minimum, sample.maximum);
        sample.value = std::clamp(sample.value, sample.minimum, sample.maximum);
        samples_[key(sample.objectId, sample.type)] = sample;
    }

    [[nodiscard]] const SimulationFieldSample* get(
        std::uint64_t objectId,
        SimulationFieldType type) const noexcept
    {
        const auto it = samples_.find(key(objectId, type));
        return it == samples_.end() ? nullptr : &it->second;
    }

private:
    static std::uint64_t key(std::uint64_t objectId, SimulationFieldType type) noexcept
    {
        return (objectId << 8u) | static_cast<std::uint64_t>(type);
    }

    std::unordered_map<std::uint64_t, SimulationFieldSample> samples_;
};

} // namespace mir
