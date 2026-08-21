#pragma once

#include "MaterialInterface.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mir
{

struct MaterialInteractionEdge
{
    std::uint64_t interfaceId{0};
    std::uint64_t firstRegionId{0};
    std::uint64_t secondRegionId{0};
    MaterialInterfaceProcess process{MaterialInterfaceProcess::ThermalTransfer};
    Scalar coefficient{0.0};
    bool enabled{true};
};

class MaterialInteractionGraph
{
public:
    void rebuild(
        const MaterialRegionStore& regions,
        const MaterialInterfaceStore& interfaces)
    {
        edges_.clear();

        for (std::uint64_t id = 1; id <= interfaceCountHint(interfaces); ++id)
        {
            const auto* interfaceValue = interfaces.get(id);
            if (!interfaceValue || !interfaceValue->enabled)
                continue;

            const auto* first = regions.get(interfaceValue->firstRegionId);
            const auto* second = regions.get(interfaceValue->secondRegionId);
            if (!first || !second || !first->enabled || !second->enabled)
                continue;

            edges_.push_back({
                interfaceValue->interfaceId,
                interfaceValue->firstRegionId,
                interfaceValue->secondRegionId,
                interfaceValue->process,
                interfaceValue->coefficient,
                true
            });
        }
    }

    void clear() noexcept
    {
        edges_.clear();
    }

    [[nodiscard]] const std::vector<MaterialInteractionEdge>& edges() const noexcept
    {
        return edges_;
    }

    [[nodiscard]] std::vector<MaterialInteractionEdge> forRegion(
        std::uint64_t regionId) const
    {
        std::vector<MaterialInteractionEdge> result;
        for (const auto& edge : edges_)
        {
            if (edge.firstRegionId == regionId || edge.secondRegionId == regionId)
                result.push_back(edge);
        }
        return result;
    }

private:
    static std::uint64_t interfaceCountHint(const MaterialInterfaceStore& interfaces) noexcept
    {

        return static_cast<std::uint64_t>(interfaces.size());
    }

    std::vector<MaterialInteractionEdge> edges_;
};

}
