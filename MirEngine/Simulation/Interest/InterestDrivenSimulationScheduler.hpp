#pragma once

#include "../WorldSimulationScheduler.hpp"
#include "../../World/Interest/WorldInterestManager.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace mir
{

struct InterestSimulationWeights
{
    float mechanics{1.0F};
    float fluid{1.0F};
    float thermal{0.8F};
    float chemistry{0.6F};
    float aerodynamics{0.7F};
    float acoustics{0.5F};
    float rendering{1.0F};
};

struct InterestSimulationRequest
{
    std::uint64_t objectId{0};
    WorldPosition position{};
    InterestSimulationWeights weights{};
    bool enabled{true};
};

struct InterestSimulationDecision
{
    std::uint64_t objectId{0};
    float interestScore{0.0F};
    std::uint8_t priority{0};
    WorldSimulationSchedule schedule{};
};

class InterestDrivenSimulationScheduler
{
public:
    void setRequest(const InterestSimulationRequest& request)
    {
        requests_[request.objectId] = request;
    }

    void removeRequest(std::uint64_t objectId)
    {
        requests_.erase(objectId);
    }

    [[nodiscard]] InterestSimulationDecision build(
        std::uint64_t objectId,
        const WorldInterestManager& interest,
        const WorldRenderBudget& worldBudget,
        const WorldSimulationScheduler& scheduler) const
    {
        InterestSimulationDecision result{};
        result.objectId = objectId;

        const auto interestScore = interest.evaluate(objectId);
        result.interestScore = interestScore.score;
        result.priority = interestScore.priority;

        const auto requestIterator = requests_.find(objectId);
        if (requestIterator == requests_.end() || !requestIterator->second.enabled)
            return result;

        WorldRenderBudget localBudget = worldBudget;

        const float multiplier = 0.5F + interestScore.score * 1.5F;
        localBudget.physicsBudget = std::max<std::size_t>(
            1,
            static_cast<std::size_t>(worldBudget.physicsBudget * multiplier));

        result.schedule = scheduler.build(localBudget);
        return result;
    }

private:
    std::unordered_map<std::uint64_t, InterestSimulationRequest> requests_;
};

}
