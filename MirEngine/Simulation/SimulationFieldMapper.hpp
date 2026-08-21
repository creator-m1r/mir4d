#pragma once

#include "SimulationField.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

struct SimulationFieldBinding
{
    std::uint64_t objectId{0};
    SimulationFieldType fieldType{SimulationFieldType::Temperature};
    Scalar minimum{0.0};
    Scalar maximum{1.0};
};

class SimulationFieldMapper
{
public:
    void clear() noexcept { bindings_.clear(); }

    void bind(SimulationFieldBinding binding)
    {
        bindings_.push_back(binding);
    }

    void map(const SimulationFieldStore& source, SimulationFieldStore& destination) const noexcept
    {
        for (const auto& binding : bindings_)
        {
            const auto* sample = source.get(binding.objectId, binding.fieldType);
            if (!sample)
                continue;

            SimulationFieldSample mapped = *sample;
            mapped.minimum = binding.minimum;
            mapped.maximum = binding.maximum;
            destination.set(mapped);
        }
    }

    [[nodiscard]] const std::vector<SimulationFieldBinding>& bindings() const noexcept
    {
        return bindings_;
    }

private:
    std::vector<SimulationFieldBinding> bindings_;
};

} // namespace mir
