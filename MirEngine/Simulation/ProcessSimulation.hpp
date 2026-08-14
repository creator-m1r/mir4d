#pragma once

#include "ProcessGraph.hpp"
#include "SimulationFields.hpp"

namespace mir
{

class ProcessSimulation
{
public:
    void step(Scalar deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0)
            return;

        graph_.step(deltaSeconds);
        updateFields();
    }

    void start() noexcept { graph_.start(); }
    void stop() noexcept { graph_.stop(); }

    [[nodiscard]] ProcessGraph& graph() noexcept { return graph_; }
    [[nodiscard]] const ProcessGraph& graph() const noexcept { return graph_; }
    [[nodiscard]] SimulationFields& fields() noexcept { return fields_; }
    [[nodiscard]] const SimulationFields& fields() const noexcept { return fields_; }

private:
    void updateFields() noexcept
    {
        for (const auto& connection : graph_.connections())
        {
            const auto* source = graph_.state(connection.source);
            if (!source)
                continue;

            // The process state remains the authoritative low-cost runtime state.
            // Renderers and higher-fidelity solvers can consume these values as fields.
            lastFlow_ = source->flow;
            lastPressure_ = source->pressure;
            lastTemperature_ = source->temperature;
        }
    }

    ProcessGraph graph_{};
    SimulationFields fields_{};
    Scalar lastFlow_{0.0};
    Scalar lastPressure_{101325.0};
    Scalar lastTemperature_{293.15};
};

} // namespace mir
