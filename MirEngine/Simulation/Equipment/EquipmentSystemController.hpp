#pragma once

#include "EquipmentCommandProcessor.hpp"
#include "EquipmentSystem.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace mir
{

enum class EquipmentSystemControlState : std::uint8_t
{
    Stopped,
    Starting,
    Running,
    Stopping,
    EmergencyStopped
};

class EquipmentSystemController
{
public:
    [[nodiscard]] EquipmentSystemControlState state() const noexcept { return state_; }

    bool start(
        EquipmentModelStore& equipment,
        EquipmentSystem& system) noexcept
    {
        if (!system.enabled || state_ == EquipmentSystemControlState::EmergencyStopped)
            return false;

        state_ = EquipmentSystemControlState::Starting;

        // Start equipment in the declared system order.
        for (const auto equipmentId : system.equipmentIds)
        {
            const EquipmentCommandRequest request{
                equipmentId,
                EquipmentCommandType::Start,
                {},
                0.0};

            if (!processor_.execute(equipment, request))
            {
                emergencyStop(equipment, system);
                return false;
            }
        }

        system.running = true;
        state_ = EquipmentSystemControlState::Running;
        return true;
    }

    bool stop(
        EquipmentModelStore& equipment,
        EquipmentSystem& system) noexcept
    {
        if (state_ == EquipmentSystemControlState::EmergencyStopped)
            return false;

        state_ = EquipmentSystemControlState::Stopping;

        for (auto it = system.equipmentIds.rbegin(); it != system.equipmentIds.rend(); ++it)
        {
            const EquipmentCommandRequest request{
                *it,
                EquipmentCommandType::Stop,
                {},
                0.0};
            processor_.execute(equipment, request);
        }

        system.running = false;
        state_ = EquipmentSystemControlState::Stopped;
        return true;
    }

    void emergencyStop(
        EquipmentModelStore& equipment,
        EquipmentSystem& system) noexcept
    {
        for (const auto equipmentId : system.equipmentIds)
        {
            const EquipmentCommandRequest request{
                equipmentId,
                EquipmentCommandType::Stop,
                {},
                0.0};
            processor_.execute(equipment, request);
        }

        system.running = false;
        state_ = EquipmentSystemControlState::EmergencyStopped;
    }

    void resetEmergencyStop() noexcept
    {
        if (state_ == EquipmentSystemControlState::EmergencyStopped)
            state_ = EquipmentSystemControlState::Stopped;
    }

private:
    EquipmentCommandProcessor processor_{};
    EquipmentSystemControlState state_{EquipmentSystemControlState::Stopped};
};

} // namespace mir
