#include "MirEngine/Simulation/CAECampaign.hpp"
#include "MirEngine/Simulation/CAEJsonReport.hpp"

#include <cstddef>
#include <cstring>
#include <string>

extern "C" {

/// Runs a CAE multiphysics campaign defined by the text grammar accepted by
/// mir::CAECampaign::loadFromText and writes the JSON result report into the
/// caller-provided buffer. Returns false only when the definition cannot be
/// parsed or the output buffer is invalid. A campaign whose cases fail still
/// produces a valid JSON report (passed=false).
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

} // extern "C"
