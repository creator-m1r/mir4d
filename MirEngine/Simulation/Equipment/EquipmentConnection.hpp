#pragma once

#include "EquipmentModel.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace mir
{

enum class EquipmentConnectionProcess : std::uint8_t
{
    Fluid,
    Mechanical,
    Thermal,
    Electrical,
    Control
};

struct EquipmentConnection
{
    std::uint64_t id{0};
    std::uint64_t sourceEquipmentId{0};
    std::uint64_t sourcePortId{0};
    std::uint64_t targetEquipmentId{0};
    std::uint64_t targetPortId{0};
    EquipmentConnectionProcess process{EquipmentConnectionProcess::Control};
    bool enabled{true};
};

class EquipmentConnectionStore
{
public:
    bool connect(const EquipmentModelStore& equipment, EquipmentConnection connection)
    {
        if (connection.id == 0 ||
            connection.sourceEquipmentId == 0 ||
            connection.targetEquipmentId == 0 ||
            connection.sourcePortId == 0 ||
            connection.targetPortId == 0 ||
            connection.sourceEquipmentId == connection.targetEquipmentId)
            return false;

        const auto* source = equipment.get(connection.sourceEquipmentId);
        const auto* target = equipment.get(connection.targetEquipmentId);
        if (!source || !target)
            return false;

        if (!hasPort(*source, connection.sourcePortId) ||
            !hasPort(*target, connection.targetPortId))
            return false;

        connections_[connection.id] = std::move(connection);
        return true;
    }

    void disconnect(std::uint64_t id) noexcept
    {
        connections_.erase(id);
    }

    void clear() noexcept
    {
        connections_.clear();
    }

    [[nodiscard]] const EquipmentConnection* get(std::uint64_t id) const noexcept
    {
        const auto it = connections_.find(id);
        return it == connections_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::vector<EquipmentConnection> forEquipment(
        std::uint64_t equipmentId) const
    {
        std::vector<EquipmentConnection> result;
        for (const auto& [id, connection] : connections_)
        {
            (void)id;
            if (connection.sourceEquipmentId == equipmentId ||
                connection.targetEquipmentId == equipmentId)
                result.push_back(connection);
        }
        return result;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return connections_.size();
    }

private:
    static bool hasPort(const EquipmentModel& model, std::uint64_t portId) noexcept
    {
        for (const auto& port : model.ports)
            if (port.id == portId && port.enabled)
                return true;
        return false;
    }

    std::unordered_map<std::uint64_t, EquipmentConnection> connections_;
};

}
