#include "StepImporter.hpp"
#include "OcctBridge.hpp"

namespace mir::io::step
{

ImportResult StepImporter::importFile(
    const std::string& path,
    const ImportOptions&) const
{
    ImportResult result;
    result.format = Format::Step;
    result.sourcePath = path;

    if (!OcctBridge::available())
    {
        result.error = OcctBridge::availabilityMessage();
        return result;
    }

    // The exact OCCT -> MIR B-Rep/tessellation mapping is deliberately not
    // faked here. This seam is the only place where STEP implementation should
    // be introduced once the project links an approved OpenCASCADE build.
    result.error = "OCCT is enabled, but STEP shape mapping is not implemented yet."
                   " No partial or lossy STEP import is exposed as canonical geometry.";
    return result;
}

} // namespace mir::io::step
