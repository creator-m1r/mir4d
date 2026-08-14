#pragma once

#include "WorldTypes.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace mir
{

class WorldObject
{
public:
    using Id = std::uint64_t;

    constexpr WorldObject(Id id = 0, WorldObjectType type = WorldObjectType::Marker) noexcept
        : id_(id), type_(type)
    {
    }

    [[nodiscard]] constexpr Id id() const noexcept { return id_; }
    [[nodiscard]] constexpr WorldObjectType type() const noexcept { return type_; }

    void setName(std::string name) { name_ = std::move(name); }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    void setState(const WorldObjectState& state) noexcept { state_ = state; }
    [[nodiscard]] WorldObjectState& state() noexcept { return state_; }
    [[nodiscard]] const WorldObjectState& state() const noexcept { return state_; }

    void setMaterial(const MaterialProperties& material) { material_ = material; }
    [[nodiscard]] const MaterialProperties& material() const noexcept { return material_; }

private:
    Id id_{0};
    WorldObjectType type_{WorldObjectType::Marker};
    std::string name_{};
    WorldObjectState state_{};
    MaterialProperties material_{};
};

} // namespace mir
