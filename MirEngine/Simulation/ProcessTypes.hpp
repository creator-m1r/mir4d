#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Math/Vector/Vector.hpp"
#include "../World/WorldObject.hpp"

#include <cstdint>
#include <string>

namespace mir
{

enum class ProcessPortType : std::uint8_t
{
    Material,
    Energy,
    Motion,
    Signal,
    Air
};

struct ProcessState
{
    bool enabled{true};
    bool running{false};
    Scalar efficiency{1.0};
    Scalar power{0.0};
    Scalar speed{0.0};
    Scalar flow{0.0};
    Scalar pressure{0.0};
    Scalar temperature{293.15};
};

struct ProcessPort
{
    std::string name{};
    ProcessPortType type{ProcessPortType::Material};
    WorldObject::Id object{0};
    Vector3 localPosition{};
};

struct ProcessConnection
{
    WorldObject::Id source{0};
    WorldObject::Id target{0};
    ProcessPortType type{ProcessPortType::Material};
    Scalar capacity{1.0};
};

}
