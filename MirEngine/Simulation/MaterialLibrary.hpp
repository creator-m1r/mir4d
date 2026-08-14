#pragma once

#include "MaterialConfigParser.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mir
{

class MaterialLibrary
{
public:
    [[nodiscard]] bool registerMaterial(MaterialProperties material)
    {
        if (material.name.empty())
            return false;

        materials_[material.name] = std::move(material);
        return true;
    }

    [[nodiscard]] bool loadMaterial(
        std::string_view text,
        std::string_view fallbackName = {})
    {
        MaterialProperties material;
        if (!fallbackName.empty())
            material.name = std::string(fallbackName);

        if (!MaterialConfigParser::parse(text, material))
            return false;

        return registerMaterial(std::move(material));
    }

    [[nodiscard]] const MaterialProperties* find(std::string_view name) const noexcept
    {
        const auto it = materials_.find(std::string(name));
        return it == materials_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] MaterialProperties* find(std::string_view name) noexcept
    {
        const auto it = materials_.find(std::string(name));
        return it == materials_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] bool contains(std::string_view name) const noexcept
    {
        return find(name) != nullptr;
    }

    bool remove(std::string_view name) noexcept
    {
        return materials_.erase(std::string(name)) != 0;
    }

    void clear() noexcept
    {
        materials_.clear();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return materials_.size();
    }

    [[nodiscard]] std::vector<std::string> names() const
    {
        std::vector<std::string> result;
        result.reserve(materials_.size());

        for (const auto& [name, _] : materials_)
            result.push_back(name);

        std::sort(result.begin(), result.end());
        return result;
    }

private:
    std::unordered_map<std::string, MaterialProperties> materials_;
};

} // namespace mir
