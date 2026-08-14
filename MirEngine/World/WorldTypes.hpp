#pragma once

#include "../Core/Types/Scalar.hpp"
#include "../Math/Vector/Vector.hpp"

#include <cstdint>
#include <string>

namespace mir
{

enum class WorldMode : std::uint8_t
{
    Explore,
    Design,
    Simulation,
    Inspection
};

enum class WorldObjectType : std::uint8_t
{
    Terrain,
    Water,
    Plant,
    Building,
    Machine,
    Part,
    Material,
    Tool,
    Marker
};

struct WorldObjectState
{
    Vector3 position{};
    Vector3 rotation{};
    Vector3 scale{1.0, 1.0, 1.0};
    bool visible{true};
    bool selectable{true};
};

struct MaterialProperties
{
    std::string name{};
    std::string formula{};
    Scalar density{0.0};
    Scalar temperature{293.15};
    Scalar pressure{101325.0};
};

struct WorldSettings
{
    WorldMode mode{WorldMode::Explore};
    bool physicsEnabled{true};
    bool acousticsEnabled{true};
    bool chemistryInspectionEnabled{true};
    bool adaptiveQuality{true};
};

} // namespace mir
