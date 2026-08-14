#pragma once

#include "../SimulationTypes.hpp"
#include "../SimulationState.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace mir
{

enum class EquipmentPortType : std::uint8_t
{
    FluidInput,
    FluidOutput,
    MechanicalInput,
    MechanicalOutput,
    ThermalInput,
    ThermalOutput,
    ElectricalInput,
    ElectricalOutput,
    ControlInput,
    ControlOutput
};

struct EquipmentPort
{
    std::uint64_t id{0};
    std::string name;
    EquipmentPortType type{EquipmentPortType::ControlInput};
    bool enabled{true};
};

struct EquipmentParameter
{
    std::string name;
    Scalar value{0.0};
    std::string unit;
};

struct EquipmentCommand
{
    std::string name;
    Scalar value{0.0};
    bool enabled{true};
};

struct EquipmentModel
{
    std::uint64_t id{0};
    std::string type;
    std::string name;

    std::vector<EquipmentPort> ports;
    std::vector<EquipmentParameter> parameters;
    std::vector<EquipmentCommand> commands;

    SimulationState state{};
    bool enabled{true};
};

class EquipmentModelStore
{
public:
    bool add(EquipmentModel model)
    {
        if (model.id == 0 || model.type.empty() || model.name.empty())
            return false;
        models_[model.id] = std::move(model);
        return true;
    }

    void remove(std::uint64_t id) noexcept { models_.erase(id); }
    void clear() noexcept { models_.clear(); }

    [[nodiscard]] EquipmentModel* get(std::uint64_t id) noexcept
    {
        const auto it = models_.find(id);
        return it == models_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const EquipmentModel* get(std::uint64_t id) const noexcept
    {
        const auto it = models_.find(id);
        return it == models_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::size_t size() const noexcept { return models_.size(); }

private:
    std::unordered_map<std::uint64_t, EquipmentModel> models_;
};

} // namespace mir
