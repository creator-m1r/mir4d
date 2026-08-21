#pragma once

#include "MechanicsTypes.hpp"

#include <cstdint>
#include <string>

namespace mir
{

class Mechanism
{
public:
    using Id = std::uint64_t;

    constexpr explicit Mechanism(Id id = 0, MechanismType type = MechanismType::Generic) noexcept
        : id_(id), type_(type)
    {
    }

    [[nodiscard]] constexpr Id id() const noexcept { return id_; }
    [[nodiscard]] constexpr MechanismType type() const noexcept { return type_; }

    void setName(std::string name) { name_ = std::move(name); }
    [[nodiscard]] const std::string& name() const noexcept { return name_; }

    void setAngularState(const AngularState& state) noexcept { angular_ = state; }
    [[nodiscard]] const AngularState& angularState() const noexcept { return angular_; }

    void setLinearState(const LinearState& state) noexcept { linear_ = state; }
    [[nodiscard]] const LinearState& linearState() const noexcept { return linear_; }

    [[nodiscard]] bool isValid() const noexcept
    {
        return id_ != 0 && angular_.isValid() && linear_.isValid();
    }

private:
    Id id_{0};
    MechanismType type_{MechanismType::Generic};
    std::string name_{};
    AngularState angular_{};
    LinearState linear_{};
};

}
