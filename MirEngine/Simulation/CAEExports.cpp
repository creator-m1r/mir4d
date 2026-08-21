#include "MirEngine/Simulation/CAECampaign.hpp"
#include "MirEngine/Simulation/CAEJsonReport.hpp"

#include <cstddef>
#include <cstring>
#include <string>

extern "C" {

bool MirEngineRunCAECampaign(const char* definitionText, char* outJson, size_t outCapacity)
{
    if (outJson == nullptr || outCapacity == 0)
        return false;

    mir::CAECampaign campaign;
    if (!campaign.loadFromText(definitionText != nullptr ? definitionText : ""))
        return false;

    campaign.run();

    const std::string report = campaign.result().toJson().toString();
    const size_t writeBytes = report.size() < outCapacity ? report.size() : outCapacity - 1;
    std::memcpy(outJson, report.data(), writeBytes);
    outJson[writeBytes] = '\0';
    return true;
}

}
