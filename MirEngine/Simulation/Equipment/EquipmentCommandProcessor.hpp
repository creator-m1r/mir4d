#pragma once

#include "EquipmentModel.hpp"

#include <algorithm>
#include <cstdint>
#include <string_view>

namespace mir
{

enum class EquipmentCommandType : std::uint8_t
{
    Start,
    Stop,
    SetParameter,
    Open,
    Close
};

struct EquipmentCommandRequest
{
    std::uint64_t equipmentId{0};
    EquipmentCommandType type{EquipmentCommandType::Start};
    std::string_view parameter{};
    Scalar value{0.0};
};

class EquipmentCommandProcessor
{
public:
    [[nodiscard]] bool execute(
        EquipmentModelStore& equipment,
        const EquipmentCommandRequest& request) const noexcept
    {
        auto* model = equipment.get(request.equipmentId);
        if (!model || !model->enabled)
            return false;

        switch (request.type)
        {
        case EquipmentCommandType::Start:
            return command(model, "START");

        case EquipmentCommandType::Stop:
            return command(model, "STOP");

        case EquipmentCommandType::SetParameter:
            return setParameter(*model, request.parameter, request.value);

        case EquipmentCommandType::Open:
            return setParameter(*model, "opening", 1.0);

        case EquipmentCommandType::Close:
            return setParameter(*model, "opening", 0.0);
        }

        return false;
    }

private:
    static bool command(EquipmentModel* model, std::string_view name) noexcept
    {
        for (const auto& item : model->commands)
        {
            if (item.name == name && item.enabled)
            {
                model->state.running = name == "START";
                model->state.paused = false;
                return true;
            }
        }
        return false;
    }

    static bool setParameter(
        EquipmentModel& model,
        std::string_view name,
        Scalar value) noexcept
    {
        for (auto& parameter : model.parameters)
        {
            if (parameter.name == name)
            {
                parameter.value = value;
                return true;
            }
        }
        return false;
    }
};

} // namespace mir
