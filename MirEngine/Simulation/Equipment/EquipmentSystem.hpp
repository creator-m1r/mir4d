#pragma once

#include "EquipmentModel.hpp"
#include "EquipmentConnection.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir
{

struct EquipmentSystem
{
    std::uint64_t id{0};
    std::string name;
    std::vector<std::uint64_t> equipmentIds;
    bool enabled{true};
    bool running{false};
};

class EquipmentSystemStore
{
public:
    bool add(EquipmentSystem system)
    {
        if (system.id == 0 || system.name.empty())
            return false;
        systems_[system.id] = std::move(system);
        return true;
    }

    bool addEquipment(
        const EquipmentModelStore& equipment,
        std::uint64_t systemId,
        std::uint64_t equipmentId)
    {
        auto* system = get(systemId);
        if (!system || !equipment.get(equipmentId))
            return false;

        for (const auto id : system->equipmentIds)
            if (id == equipmentId)
                return true;

        system->equipmentIds.push_back(equipmentId);
        return true;
    }

    bool removeEquipment(std::uint64_t systemId, std::uint64_t equipmentId) noexcept
    {
        auto* system = get(systemId);
        if (!system)
            return false;

        const auto oldSize = system->equipmentIds.size();
        system->equipmentIds.erase(
            std::remove(
                system->equipmentIds.begin(),
                system->equipmentIds.end(),
                equipmentId),
            system->equipmentIds.end());
        return system->equipmentIds.size() != oldSize;
    }

    void start(std::uint64_t systemId) noexcept
    {
        if (auto* system = get(systemId))
        {
            system->running = system->enabled;
        }
    }

    void stop(std::uint64_t systemId) noexcept
    {
        if (auto* system = get(systemId))
            system->running = false;
    }

    [[nodiscard]] EquipmentSystem* get(std::uint64_t id) noexcept
    {
        const auto it = systems_.find(id);
        return it == systems_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const EquipmentSystem* get(std::uint64_t id) const noexcept
    {
        const auto it = systems_.find(id);
        return it == systems_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return systems_.size();
    }

private:
    std::unordered_map<std::uint64_t, EquipmentSystem> systems_;
};

} // namespace mir
