#pragma once

#include "ProcessGraph.hpp"

#include <algorithm>

namespace mir
{

class ConnectionSolver
{
public:
    void step(ProcessGraph& graph, Scalar deltaSeconds) const noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        for (const auto& connection : graph.connections())
        {
            auto* source = graph.state(connection.source);
            auto* target = graph.state(connection.target);

            if (!source || !target || !source->enabled || !source->running || !target->enabled)
                continue;

            const Scalar capacity = std::max(0.0, connection.capacity);
            const Scalar available = std::max(0.0, source->flow);
            const Scalar transferred = std::min(available, available * capacity) * deltaSeconds;

            target->flow += transferred;
            target->pressure += (source->pressure - target->pressure) * 0.02 * capacity;
            target->temperature +=
                (source->temperature - target->temperature) * 0.02 * capacity;
            target->efficiency = std::clamp(target->efficiency, 0.0, 1.0);
        }
    }
};

} // namespace mir
