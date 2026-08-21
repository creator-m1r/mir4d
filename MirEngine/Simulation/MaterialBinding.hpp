#pragma once

#include "MaterialLibrary.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace mir
{

struct MaterialBinding
{
    std::uint64_t objectId{0};
    std::string materialName;
    float blend{1.0f};
    bool enabled{true};
};

class MaterialBindingStore
{
public:
    bool bind(std::uint64_t objectId, std::string_view materialName, float blend = 1.0f) {
        if (objectId == 0 || materialName.empty()) return false;
        bindings_[objectId] = MaterialBinding{objectId, std::string(materialName), blend, true};
        return true;
    }

    void unbind(std::uint64_t objectId) noexcept { bindings_.erase(objectId); }

    [[nodiscard]] const MaterialBinding* get(std::uint64_t objectId) const noexcept {
        const auto it = bindings_.find(objectId);
        return it == bindings_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const MaterialProperties* resolve(
        std::uint64_t objectId, const MaterialLibrary& library) const noexcept {
        const auto* binding = get(objectId);
        return binding && binding->enabled ? library.find(binding->materialName) : nullptr;
    }

private:
    std::unordered_map<std::uint64_t, MaterialBinding> bindings_;
};

} // namespace mir
