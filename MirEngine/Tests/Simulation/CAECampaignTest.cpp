#include "MirEngine/Simulation/CAECampaign.hpp"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>

int main()
{
    mir::WorldMaterialDescriptor material{};
    material.temperature = 350.0;
    material.density = 0.0;

    mir::SimulationState initial{};
    initial.flowRate = 5.0;
    initial.composition["reactant"] = 1.0;

    mir::CAECampaignCase nominal;
    nominal.name = "nominal";
    nominal.material = material;
    nominal.initial = initial;
    nominal.duration = 5.0;
    nominal.timeStep = 0.1;
    nominal.criteria.push_back({mir::CAEMetric::Temperature, 0.0, 400.0});
    nominal.criteria.push_back({mir::CAEMetric::Density, 500.0, 2000.0});
    nominal.criteria.push_back({mir::CAEMetric::Velocity, 0.0, 1.0});
    nominal.criteria.push_back({mir::CAEMetric::Composition, 0.0, 1.0, "product"});
    nominal.criteria.push_back({mir::CAEMetric::Stress, 0.0, 1.0e9});

    mir::CAECampaignCase overheat;
    overheat.name = "overheat";
    overheat.material = material;
    overheat.initial = initial;
    overheat.duration = 5.0;
    overheat.timeStep = 0.1;
    overheat.criteria.push_back({mir::CAEMetric::Temperature, 0.0, 300.0});

    mir::CAECampaign campaign;
    campaign.addCase(nominal);
    campaign.addCase(overheat);
    assert(campaign.size() == 2);

    const bool overall = campaign.run();
    const auto& result = campaign.result();

    assert(!overall);
    assert(!result.passed);
    assert(result.cases.size() == 2);
    assert(result.cases[0].first == "nominal");
    assert(result.cases[0].second.passed);
    assert(result.cases[1].first == "overheat");
    assert(!result.cases[1].second.passed);

    const std::string definition =
        "case nominal\n"
        "material temperature 350\n"
        "material density 0\n"
        "material youngModulus 2e11\n"
        "initial flowRate 5\n"
        "initial composition reactant 1\n"
        "duration 5\n"
        "timeStep 0.1\n"
        "criterion temperature 0 400\n"
        "criterion stress 0 1e9\n"
        "criterion composition 0 1 product\n"
        "\n"
        "case overheat\n"
        "material temperature 350\n"
        "initial flowRate 5\n"
        "criterion temperature 0 300\n";

    mir::CAECampaign parsed;
    assert(parsed.loadFromText(definition));
    assert(parsed.size() == 2);
    const bool parsedOverall = parsed.run();
    const auto& parsedResult = parsed.result();
    assert(!parsedOverall);
    assert(parsedResult.cases[0].first == "nominal");
    assert(parsedResult.cases[0].second.passed);
    assert(!parsedResult.cases[1].second.passed);

    assert(parsed.saveReportToFile("/tmp/cae_campaign_report.txt"));
    std::ifstream reportFile("/tmp/cae_campaign_report.txt");
    assert(reportFile.good());
    std::string reportLine;
    bool found = false;
    while (std::getline(reportFile, reportLine))
        if (reportLine.find("CAE multiphysics campaign:") != std::string::npos)
            found = true;
    assert(found);
    std::remove("/tmp/cae_campaign_report.txt");

    return 0;
}
