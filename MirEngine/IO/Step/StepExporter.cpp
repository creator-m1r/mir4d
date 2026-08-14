#include "StepExporter.hpp"
#include "OcctBridge.hpp"

namespace mir::io::step
{

ExportResult StepExporter::exportTo(
    const std::string& path,
    const mir4d::Document&,
    const ExportOptions&)
{
    ExportResult result;
    result.format = Format::Step;
    result.targetPath = path;

    if (!OcctBridge::available())
    {
        result.error = OcctBridge::availabilityMessage();
        return result;
    }

    result.error = "OCCT is enabled, but MIR B-Rep to STEP mapping is not implemented yet."
                   " No mesh-to-solid conversion is performed implicitly.";
    return result;
}

} // namespace mir::io::step
