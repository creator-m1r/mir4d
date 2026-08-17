#pragma once

#include "SimulationWorld.hpp"
#include "../World/WorldTypes.hpp"
#include "CAEJsonReport.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace mir
{

enum class CAEMetric : std::uint8_t
{
    Temperature,
    Pressure,
    Density,
    Velocity,
    Drag,
    Acoustic,
    Composition,
    Stress
};

struct CAECriterion
{
    CAEMetric metric{CAEMetric::Temperature};
    Scalar min{std::numeric_limits<Scalar>::lowest()};
    Scalar max{std::numeric_limits<Scalar>::max()};
    std::string compositionKey{"product"};
    bool enabled{true};
};

struct CAETestResult
{
    bool passed{false};
    Scalar minTemperature{0.0};
    Scalar maxTemperature{0.0};
    Scalar minPressure{0.0};
    Scalar maxPressure{0.0};
    Scalar minDensity{0.0};
    Scalar maxDensity{0.0};
    Scalar maxVelocity{0.0};
    Scalar maxDrag{0.0};
    Scalar maxAcoustic{0.0};
    Scalar maxConcentration{0.0};
    Scalar minStress{0.0};
    Scalar maxStress{0.0};
    std::string compositionKey{"product"};
    std::vector<std::string> failures{};

    [[nodiscard]] std::string report() const
    {
        std::string out = "CAE multiphysics test: ";
        out += passed ? "PASSED" : "FAILED";
        out += "\n";
        out += "  temperature  min=" + std::to_string(minTemperature)
             + " max=" + std::to_string(maxTemperature) + "\n";
        out += "  pressure     min=" + std::to_string(minPressure)
             + " max=" + std::to_string(maxPressure) + "\n";
        out += "  density      min=" + std::to_string(minDensity)
             + " max=" + std::to_string(maxDensity) + "\n";
        out += "  velocity     max=" + std::to_string(maxVelocity) + "\n";
        out += "  drag         max=" + std::to_string(maxDrag) + "\n";
        out += "  acoustic     max=" + std::to_string(maxAcoustic) + "\n";
        out += "  concentration[" + compositionKey + "] max="
             + std::to_string(maxConcentration) + "\n";
        out += "  stress       min=" + std::to_string(minStress)
             + " max=" + std::to_string(maxStress) + "\n";
        for (const auto& failure : failures)
            out += "  FAIL: " + failure + "\n";
        return out;
    }

    bool saveToFile(std::string_view path) const
    {
        std::ofstream out{std::string(path)};
        if (!out)
            return false;
        out << report();
        return static_cast<bool>(out);
    }

    [[nodiscard]] JsonValue toJson() const
    {
        JsonValue telemetry = JsonValue::object();
        JsonValue temperature = JsonValue::object();
        temperature.set("min", minTemperature);
        temperature.set("max", maxTemperature);
        telemetry.set("temperature", temperature);

        JsonValue pressure = JsonValue::object();
        pressure.set("min", minPressure);
        pressure.set("max", maxPressure);
        telemetry.set("pressure", pressure);

        JsonValue density = JsonValue::object();
        density.set("min", minDensity);
        density.set("max", maxDensity);
        telemetry.set("density", density);

        JsonValue stress = JsonValue::object();
        stress.set("min", minStress);
        stress.set("max", maxStress);
        telemetry.set("stress", stress);

        telemetry.set("maxVelocity", maxVelocity);
        telemetry.set("maxDrag", maxDrag);
        telemetry.set("maxAcoustic", maxAcoustic);
        telemetry.set("maxConcentration", maxConcentration);

        JsonValue failuresNode = JsonValue::array();
        for (const auto& failure : failures)
            failuresNode.push(JsonValue(failure));

        JsonValue root = JsonValue::object();
        root.set("passed", passed);
        root.set("compositionKey", compositionKey);
        root.set("telemetry", telemetry);
        root.set("failures", failuresNode);
        return root;
    }

    bool saveJsonToFile(std::string_view path) const
    {
        std::ofstream out{std::string(path)};
        if (!out)
            return false;
        out << toJson().toString();
        return static_cast<bool>(out);
    }
};

class CAETest
{
public:
    void setTimeStep(Scalar dt) noexcept { dt_ = dt; }
    void setDuration(Scalar seconds) noexcept { duration_ = seconds; }
    void setMaterial(const WorldMaterialDescriptor& material) noexcept { material_ = material; }
    void setInitialState(const SimulationState& state) noexcept { initial_ = state; }
    void setLoad(Scalar heatFlux, Scalar pressureLoad, bool fixed = false) noexcept
    {
        initial_.heatFlux = heatFlux;
        initial_.pressureLoad = pressureLoad;
        initial_.fixed = fixed;
    }
    void addCriterion(CAEMetric metric, Scalar min, Scalar max, std::string compositionKey = "product")
    {
        criteria_.push_back({metric, min, max, std::move(compositionKey), true});
    }

    bool run() noexcept
    {
        World world;
        auto specimen = world.create(WorldObjectType::Part, "specimen");
        specimen->setMaterial(material_);

        SimulationWorld simulation;
        simulation.settings().running = true;
        simulation.start();

        simulation.states().state(specimen->id()) = initial_;

        result_ = CAETestResult{};
        result_.compositionKey = criteria_.empty() ? "product" : criteria_.front().compositionKey;

        const Scalar inf = std::numeric_limits<Scalar>::max();
        const Scalar neg = std::numeric_limits<Scalar>::lowest();
        result_.minTemperature = result_.minPressure = result_.minDensity = inf;
        result_.maxTemperature = result_.maxPressure = result_.maxDensity = neg;
        result_.maxVelocity = result_.maxDrag = result_.maxAcoustic = result_.maxConcentration = neg;
        result_.minStress = inf;
        result_.maxStress = neg;

        Scalar elapsed = 0.0;
        while (elapsed < duration_ - 1e-9)
        {
            simulation.step(world, dt_);
            if (const auto* state = simulation.states().find(specimen->id()))
                record(*state);
            elapsed += dt_;
        }

        evaluate();
        return result_.passed;
    }

    [[nodiscard]] const CAETestResult& result() const noexcept { return result_; }

private:
    void record(const SimulationState& state) noexcept
    {
        const Scalar speed = std::sqrt(
            state.velocity.x * state.velocity.x
            + state.velocity.y * state.velocity.y
            + state.velocity.z * state.velocity.z);

        result_.minTemperature = std::min(result_.minTemperature, state.temperature);
        result_.maxTemperature = std::max(result_.maxTemperature, state.temperature);
        result_.minPressure = std::min(result_.minPressure, state.pressure);
        result_.maxPressure = std::max(result_.maxPressure, state.pressure);
        result_.minDensity = std::min(result_.minDensity, state.density);
        result_.maxDensity = std::max(result_.maxDensity, state.density);
        result_.maxVelocity = std::max(result_.maxVelocity, speed);
        result_.maxDrag = std::max(result_.maxDrag, state.aerodynamicDrag);
        result_.maxAcoustic = std::max(result_.maxAcoustic, state.acousticLevel);

        result_.minStress = std::min(result_.minStress, state.stress);
        result_.maxStress = std::max(result_.maxStress, state.stress);

        const auto it = state.composition.find(result_.compositionKey);
        if (it != state.composition.end())
            result_.maxConcentration = std::max(result_.maxConcentration, it->second);
    }

    void evaluate() noexcept
    {
        result_.passed = true;
        for (const auto& criterion : criteria_)
        {
            if (!criterion.enabled)
                continue;

            Scalar seriesMin = 0.0;
            Scalar seriesMax = 0.0;
            switch (criterion.metric)
            {
                case CAEMetric::Temperature:
                    seriesMin = result_.minTemperature; seriesMax = result_.maxTemperature; break;
                case CAEMetric::Pressure:
                    seriesMin = result_.minPressure; seriesMax = result_.maxPressure; break;
                case CAEMetric::Density:
                    seriesMin = result_.minDensity; seriesMax = result_.maxDensity; break;
                case CAEMetric::Velocity:
                    seriesMin = 0.0; seriesMax = result_.maxVelocity; break;
                case CAEMetric::Drag:
                    seriesMin = 0.0; seriesMax = result_.maxDrag; break;
                case CAEMetric::Acoustic:
                    seriesMin = 0.0; seriesMax = result_.maxAcoustic; break;
                case CAEMetric::Composition:
                    seriesMin = 0.0; seriesMax = result_.maxConcentration; break;
                case CAEMetric::Stress:
                    seriesMin = result_.minStress; seriesMax = result_.maxStress; break;
            }

            if (seriesMin < criterion.min || seriesMax > criterion.max)
            {
                result_.passed = false;
                result_.failures.push_back(metricName(criterion.metric)
                    + " out of [" + std::to_string(criterion.min) + ", "
                    + std::to_string(criterion.max) + "] min=" + std::to_string(seriesMin)
                    + " max=" + std::to_string(seriesMax));
            }
        }
    }

    static std::string metricName(CAEMetric metric) noexcept
    {
        switch (metric)
        {
            case CAEMetric::Temperature: return "temperature";
            case CAEMetric::Pressure: return "pressure";
            case CAEMetric::Density: return "density";
            case CAEMetric::Velocity: return "velocity";
            case CAEMetric::Drag: return "drag";
            case CAEMetric::Acoustic: return "acoustic";
            case CAEMetric::Composition: return "composition";
            case CAEMetric::Stress: return "stress";
        }
        return "unknown";
    }

    Scalar dt_{0.1};
    Scalar duration_{5.0};
    WorldMaterialDescriptor material_{};
    SimulationState initial_{};
    std::vector<CAECriterion> criteria_{};
    CAETestResult result_{};
};

} // namespace mir
