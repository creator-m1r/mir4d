#pragma once

#include "Spatial/SpatialWorldPartition.hpp"
#include "Streaming/WorldStreamingManager.hpp"
#include "Interest/WorldInterestManager.hpp"
#include "../Simulation/WorldSimulationScheduler.hpp"
#include "../Simulation/Interest/InterestDrivenSimulationScheduler.hpp"
#include "../Rendering/Particles/ParticleLODSystem.hpp"
#include "../Rendering/WorldRenderBudget.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

class MirWorldRuntime
{
public:
    explicit MirWorldRuntime(
        SpatialPartitionSettings spatialSettings = {},
        WorldStreamingBudget streamingBudget = {})
        : partition_(spatialSettings),
          streaming_(streamingBudget)
    {
    }

    void update(
        WorldPosition cameraPosition,
        const InterestCamera& camera,
        float frameTimeMs)
    {
        camera_ = camera;
        interests_.setCamera(camera_);

        streamingResult_ = streaming_.update(partition_, cameraPosition);
        renderBudget_.adapt(frameTimeMs);
        schedule_ = scheduler_.build(renderBudget_);
    }

    void setObjectInterest(
        std::uint64_t objectId,
        const WorldInterestSource& source)
    {
        interests_.setSource(objectId, source);
    }

    void setSimulationRequest(const InterestSimulationRequest& request)
    {
        interestScheduler_.setRequest(request);
    }

    [[nodiscard]] InterestSimulationDecision simulationFor(
        std::uint64_t objectId) const
    {
        return interestScheduler_.build(
            objectId,
            interests_,
            renderBudget_,
            scheduler_);
    }

    [[nodiscard]] SpatialWorldPartition& partition() noexcept
    {
        return partition_;
    }

    [[nodiscard]] const SpatialWorldPartition& partition() const noexcept
    {
        return partition_;
    }

    [[nodiscard]] WorldInterestManager& interests() noexcept
    {
        return interests_;
    }

    [[nodiscard]] const WorldInterestManager& interests() const noexcept
    {
        return interests_;
    }

    [[nodiscard]] const WorldStreamingResult& streamingResult() const noexcept
    {
        return streamingResult_;
    }

    [[nodiscard]] const WorldRenderBudget& renderBudget() const noexcept
    {
        return renderBudget_;
    }

    [[nodiscard]] const WorldSimulationSchedule& schedule() const noexcept
    {
        return schedule_;
    }

private:
    InterestCamera camera_{};

    SpatialWorldPartition partition_;
    WorldStreamingManager streaming_;
    WorldInterestManager interests_;

    WorldRenderBudget renderBudget_{};
    WorldSimulationScheduler scheduler_{};
    InterestDrivenSimulationScheduler interestScheduler_{};
    ParticleLODSystem particleLOD_{};

    WorldStreamingResult streamingResult_{};
    WorldSimulationSchedule schedule_{};
};

} // namespace mir
