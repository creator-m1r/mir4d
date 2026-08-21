#pragma once

#include "ConnectionSolver.hpp"
#include "SimulationMaterials.hpp"

namespace mir
{

struct ProcessMaterialTransfer
{
    WorldObject::Id source{0};
    WorldObject::Id target{0};
    SimulationMaterial material{};
    Scalar amount{0.0};
    Scalar pressure{0.0};
    Scalar temperature{293.15};
};

class ProcessMaterialTransferSystem
{
public:
    void step(ProcessGraph& graph, Scalar deltaSeconds) noexcept
    {
        solver_.step(graph, deltaSeconds);
    }

    void registerMaterial(const SimulationMaterial& material)
    {
        materials_.registerMaterial(material);
    }

    [[nodiscard]] SimulationMaterials& materials() noexcept { return materials_; }
    [[nodiscard]] const SimulationMaterials& materials() const noexcept { return materials_; }

private:
    ConnectionSolver solver_{};
    SimulationMaterials materials_{};
};

} // namespace mir
