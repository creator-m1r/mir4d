#pragma once

#include "CAETest.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace mir
{

inline bool caeMetricFromString(std::string_view name, CAEMetric& metric) noexcept
{
    if (name == "temperature") metric = CAEMetric::Temperature;
    else if (name == "pressure") metric = CAEMetric::Pressure;
    else if (name == "density") metric = CAEMetric::Density;
    else if (name == "velocity") metric = CAEMetric::Velocity;
    else if (name == "drag") metric = CAEMetric::Drag;
    else if (name == "acoustic") metric = CAEMetric::Acoustic;
    else if (name == "composition") metric = CAEMetric::Composition;
    else if (name == "stress") metric = CAEMetric::Stress;
    else return false;
    return true;
}

struct CAECampaignCase
{
    std::string name{"case"};
    WorldMaterialDescriptor material{};
    SimulationState initial{};
    Scalar duration{5.0};
    Scalar timeStep{0.1};
    Scalar heatFlux{0.0};
    Scalar pressureLoad{0.0};
    bool fixed{false};
    std::vector<CAECriterion> criteria{};
};

struct CAECampaignResult
{
    bool passed{false};
    std::vector<std::pair<std::string, CAETestResult>> cases{};

    [[nodiscard]] std::string report() const
    {
        std::string out = "CAE multiphysics campaign: ";
        out += passed ? "PASSED" : "FAILED";
        out += "\n";
        for (const auto& entry : cases)
        {
            out += "=== " + entry.first + (entry.second.passed ? " [PASS]" : " [FAIL]") + " ===\n";
            out += entry.second.report();
        }
        return out;
    }

    [[nodiscard]] JsonValue toJson() const
    {
        JsonValue casesNode = JsonValue::array();
        for (const auto& entry : cases)
        {
            JsonValue node = JsonValue::object();
            node.set("name", entry.first);
            node.set("passed", entry.second.passed);
            node.set("result", entry.second.toJson());
            casesNode.push(node);
        }

        JsonValue root = JsonValue::object();
        root.set("passed", passed);
        root.set("cases", casesNode);
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

class CAECampaign
{
public:
    void addCase(CAECampaignCase config) noexcept
    {
        cases_.push_back(std::move(config));
    }

    [[nodiscard]] std::size_t size() const noexcept { return cases_.size(); }

    bool loadFromText(std::string_view text) noexcept
    {
        cases_.clear();

        CAECampaignCase current{};
        bool hasCase = false;
        std::istringstream stream{std::string(text)};
        std::string line;

        const auto trim = [](std::string& value)
        {
            while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
                value.erase(value.begin());
            while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
                value.pop_back();
        };

        while (std::getline(stream, line))
        {
            trim(line);
            if (line.empty())
            {
                if (hasCase)
                {
                    cases_.push_back(std::move(current));
                    hasCase = false;
                }
                continue;
            }

            std::istringstream parser{line};
            std::string command;
            parser >> command;

            if (command == "case")
            {
                if (hasCase)
                    cases_.push_back(std::move(current));
                current = CAECampaignCase{};
                parser >> current.name;
                hasCase = true;
            }
            else if (command == "material")
            {
                std::string field; double value = 0.0;
                parser >> field >> value;
                if (field == "temperature") current.material.temperature = value;
                else if (field == "density") current.material.density = value;
                else if (field == "youngModulus") current.material.youngModulus = value;
                else if (field == "thermalExpansion") current.material.thermalExpansion = value;
                else if (field == "poissonRatio") current.material.poissonRatio = value;
                else if (field == "pressure") current.material.pressure = value;
            }
            else if (command == "initial")
            {
                std::string field; parser >> field;
                if (field == "flowRate") parser >> current.initial.flowRate;
                else if (field == "temperature") parser >> current.initial.temperature;
                else if (field == "composition")
                {
                    std::string key; double value = 0.0;
                    parser >> key >> value;
                    current.initial.composition[key] = value;
                }
            }
            else if (command == "duration") parser >> current.duration;
            else if (command == "timeStep") parser >> current.timeStep;
            else if (command == "load")
            {
                std::string kind; double value = 0.0;
                parser >> kind >> value;
                if (kind == "heatFlux") current.heatFlux = value;
                else if (kind == "pressure") current.pressureLoad = value;
                else if (kind == "fixed") current.fixed = (value != 0.0);
            }
            else if (command == "criterion")
            {
                std::string metricName; double minimum = 0.0; double maximum = 0.0;
                parser >> metricName >> minimum >> maximum;
                CAEMetric metric{};
                if (caeMetricFromString(metricName, metric))
                {
                    std::string key = "product";
                    parser >> key;
                    current.criteria.push_back({metric, minimum, maximum, key, true});
                }
            }
        }

        if (hasCase)
            cases_.push_back(std::move(current));

        return !cases_.empty();
    }

    bool run() noexcept
    {
        result_ = CAECampaignResult{};
        result_.passed = true;

        for (const auto& config : cases_)
        {
            CAETest test;
            test.setMaterial(config.material);
            test.setInitialState(config.initial);
            test.setLoad(config.heatFlux, config.pressureLoad, config.fixed);
            test.setDuration(config.duration);
            test.setTimeStep(config.timeStep);
            for (const auto& criterion : config.criteria)
                test.addCriterion(criterion.metric, criterion.min, criterion.max, criterion.compositionKey);

            const bool casePassed = test.run();
            if (!casePassed)
                result_.passed = false;

            result_.cases.emplace_back(config.name, test.result());
        }

        return result_.passed;
    }

    [[nodiscard]] const CAECampaignResult& result() const noexcept { return result_; }

    bool saveReportToFile(std::string_view path) const
    {
        std::ofstream out{std::string(path)};
        if (!out)
            return false;
        out << result_.report();
        return static_cast<bool>(out);
    }

    bool saveJsonToFile(std::string_view path) const
    {
        return result_.saveJsonToFile(path);
    }

private:
    std::vector<CAECampaignCase> cases_{};
    CAECampaignResult result_{};
};

}
