#pragma once

#include "EquipmentSystem.hpp"
#include "EquipmentCommandProcessor.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace mir
{

enum class SafetySeverity : std::uint8_t
{
    Warning,
    Critical
};

enum class SafetyParameter : std::uint8_t
{
    Pressure,
    Temperature,
    Speed,
    Level
};

struct EquipmentSafetyLimit
{
    std::uint64_t equipmentId{0};
    SafetyParameter parameter{SafetyParameter::Pressure};
    Scalar warningMin{-1.0e300};
    Scalar warningMax{1.0e300};
    Scalar criticalMin{-1.0e300};
    Scalar criticalMax{1.0e300};
    bool enabled{true};
};

struct EquipmentSafetyEvent
{
    std::uint64_t equipmentId{0};
    SafetyParameter parameter{SafetyParameter::Pressure};
    SafetySeverity severity{SafetySeverity::Warning};
    Scalar value{0.0};
    std::string message;
};

class EquipmentSafetySystem
{
public:
    void addLimit(const EquipmentSafetyLimit& limit)
    {
        if (limit.equipmentId != 0)
            limits_.push_back(limit);
    }

    void clearLimits() noexcept
    {
        limits_.clear();
        events_.clear();
    }

    [[nodiscard]] const std::vector<EquipmentSafetyEvent>& events() const noexcept
    {
        return events_;
    }

    [[nodiscard]] bool hasCriticalEvent() const noexcept
    {
        for (const auto& event : events_)
            if (event.severity == SafetySeverity::Critical)
                return true;
        return false;
    }

    void evaluate(
        const EquipmentModelStore& equipment,
        EquipmentSystem& system) noexcept
    {
        events_.clear();

        for (const auto& limit : limits_)
        {
            if (!limit.enabled)
                continue;

            const auto* model = equipment.get(limit.equipmentId);
            if (!model || !model->enabled)
                continue;

            const Scalar value = parameterValue(*model, limit.parameter);

            if (value < limit.criticalMin || value > limit.criticalMax)
            {
                events_.push_back({
                    limit.equipmentId,
                    limit.parameter,
                    SafetySeverity::Critical,
                    value,
                    "Critical safety limit exceeded"});
            }
            else if (value < limit.warningMin || value > limit.warningMax)
            {
                events_.push_back({
                    limit.equipmentId,
                    limit.parameter,
                    SafetySeverity::Warning,
                    value,
                    "Safety warning limit exceeded"});
            }
        }

        if (hasCriticalEvent())
        {
            for (const auto equipmentId : system.equipmentIds)
            {
                const EquipmentCommandRequest request{
                    equipmentId,
                    EquipmentCommandType::Stop,
                    {},
                    0.0};
                processor_.execute(const_cast<EquipmentModelStore&>(equipment), request);
            }
            system.running = false;
        }
    }

private:
    static Scalar parameterValue(
        const EquipmentModel& model,
        SafetyParameter parameter) noexcept
    {
        switch (parameter)
        {
        case SafetyParameter::Pressure:
            return model.state.pressure;
        case SafetyParameter::Temperature:
            return model.state.temperature;
        case SafetyParameter::Speed:
            return std::abs(model.state.velocity.x) * 1000.0;
        case SafetyParameter::Level:
            for (const auto& item : model.parameters)
                if (item.name == "level")
                    return item.value;
            return 0.0;
        }
        return 0.0;
    }

    std::vector<EquipmentSafetyLimit> limits_;
    std::vector<EquipmentSafetyEvent> events_;
    EquipmentCommandProcessor processor_{};
};

}
