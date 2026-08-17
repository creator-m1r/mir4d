#include "MirEngine/Simulation/CAECampaign.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
    bool fileContains(std::string_view path, std::string_view token)
    {
        std::ifstream in{std::string(path)};
        if (!in)
            return false;
        std::ostringstream buffer;
        buffer << in.rdbuf();
        return buffer.str().find(token) != std::string::npos;
    }
}

int main()
{
    mir::SimulationState initial;
    initial.flowRate = 5.0;
    initial.composition["reactant"] = 1.0;

    mir::CAETest test;
    test.setMaterial({"steel", "", 0.0, 350.0, 101325.0});
    test.setInitialState(initial);
    test.addCriterion(mir::CAEMetric::Temperature, 0.0, 400.0);
    test.addCriterion(mir::CAEMetric::Stress, 0.0, 1.0e9);
    assert(test.run());

    assert(test.result().saveJsonToFile("/tmp/cae_test_result.json"));
    assert(fileContains("/tmp/cae_test_result.json", "\"passed\""));
    assert(fileContains("/tmp/cae_test_result.json", "\"stress\""));
    assert(fileContains("/tmp/cae_test_result.json", "\"temperature\""));

    mir::CAECampaign campaign;
    const char* definition =
        "case hot\n"
        "  material temperature 350\n"
        "  initial flowRate 5\n"
        "  criterion temperature 0 400\n"
        "  criterion stress 0 1e9\n";
    assert(campaign.loadFromText(definition));
    assert(campaign.run());

    assert(campaign.saveJsonToFile("/tmp/cae_campaign_result.json"));
    assert(fileContains("/tmp/cae_campaign_result.json", "\"cases\""));
    assert(fileContains("/tmp/cae_campaign_result.json", "\"hot\""));

    return 0;
}
