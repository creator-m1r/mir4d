#pragma once

#include "ProcessTypes.hpp"

#include <unordered_map>
#include <vector>

namespace mir
{

class ProcessGraph
{
public:
    void setState(WorldObject::Id id, const ProcessState& state = {})
    {
        states_[id] = state;
    }

    void connect(const ProcessConnection& connection)
    {
        connections_.push_back(connection);
    }

    [[nodiscard]] ProcessState* state(WorldObject::Id id) noexcept
    {
        const auto it = states_.find(id);
        return it == states_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const std::vector<ProcessConnection>& connections() const noexcept
    {
        return connections_;
    }

    void start() noexcept
    {
        for (auto& [id, state] : states_)
            if (state.enabled)
                state.running = true;
    }

    void stop() noexcept
    {
        for (auto& [id, state] : states_)
            state.running = false;
    }

    void step(Scalar deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        for (const auto& connection : connections_)
        {
            auto* source = state(connection.source);
            auto* target = state(connection.target);
            if (!source || !target || !source->running || !target->enabled)
                continue;

            const Scalar transferred = source->flow * connection.capacity * deltaSeconds;
            target->flow += transferred;
            target->pressure += source->pressure * 0.01 * connection.capacity;
            target->temperature +=
                (source->temperature - target->temperature) * 0.01 * connection.capacity;
        }
    }

private:
    std::unordered_map<WorldObject::Id, ProcessState> states_{};
    std::vector<ProcessConnection> connections_{};
};

}
