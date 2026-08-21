#pragma once

#include "MaterialRegion.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace mir
{

enum class MaterialInterfaceProcess : std::uint8_t
{
    ThermalTransfer,
    Diffusion,
    Contact,
    ChemicalReaction
};

struct MaterialInterface
{
    std::uint64_t interfaceId{0};
    std::uint64_t firstRegionId{0};
    std::uint64_t secondRegionId{0};
    MaterialInterfaceProcess process{MaterialInterfaceProcess::ThermalTransfer};
    Scalar coefficient{0.0};
    bool enabled{true};
};

class MaterialInterfaceStore
{
public:
    bool add(MaterialInterface interfaceValue)
    {
        if (interfaceValue.interfaceId == 0 ||
            interfaceValue.firstRegionId == 0 ||
            interfaceValue.secondRegionId == 0 ||
            interfaceValue.firstRegionId == interfaceValue.secondRegionId)
            return false;

        interfaceValue.coefficient = std::max(0.0, interfaceValue.coefficient);
        interfaces_[interfaceValue.interfaceId] = std::move(interfaceValue);
        return true;
    }

    void remove(std::uint64_t interfaceId) noexcept
    {
        interfaces_.erase(interfaceId);
    }

    void clear() noexcept
    {
        interfaces_.clear();
    }

    [[nodiscard]] const MaterialInterface* get(std::uint64_t interfaceId) const noexcept
    {
        const auto it = interfaces_.find(interfaceId);
        return it == interfaces_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return interfaces_.size();
    }

private:
    std::unordered_map<std::uint64_t, MaterialInterface> interfaces_;
};

}
