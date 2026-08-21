#pragma once

#include "MaterialBinding.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mir
{

struct MaterialRegion
{
    std::uint64_t regionId{0};
    std::uint64_t objectId{0};
    std::string name;
    std::string materialName;
    float fraction{1.0f};
    bool enabled{true};
};

class MaterialRegionStore
{
public:
    bool add(MaterialRegion region)
    {
        if (region.regionId == 0 || region.objectId == 0 || region.materialName.empty())
            return false;

        regions_[region.regionId] = std::move(region);
        return true;
    }

    void remove(std::uint64_t regionId) noexcept
    {
        regions_.erase(regionId);
    }

    void clear() noexcept
    {
        regions_.clear();
    }

    [[nodiscard]] const MaterialRegion* get(std::uint64_t regionId) const noexcept
    {
        const auto it = regions_.find(regionId);
        return it == regions_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::vector<const MaterialRegion*> forObject(std::uint64_t objectId) const
    {
        std::vector<const MaterialRegion*> result;
        for (const auto& [_, region] : regions_)
        {
            if (region.objectId == objectId && region.enabled)
                result.push_back(&region);
        }
        return result;
    }

    [[nodiscard]] const MaterialProperties* resolve(
        std::uint64_t regionId,
        const MaterialLibrary& library) const noexcept
    {
        const auto* region = get(regionId);
        return region && region->enabled ? library.find(region->materialName) : nullptr;
    }

private:
    std::unordered_map<std::uint64_t, MaterialRegion> regions_;
};

}
