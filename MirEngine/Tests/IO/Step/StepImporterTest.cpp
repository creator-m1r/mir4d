#include "MirEngine/IO/Step/StepImporter.hpp"
#include "MirEngine/IO/Step/OcctBridge.hpp"

#include <cassert>

int main()
{
    mir::io::step::StepImporter importer;
    const auto result = importer.importFile("activation-test.step");

    assert(result.format == mir::io::Format::Step);
    assert(result.sourcePath == "activation-test.step");
    assert(!result.error.empty());
    assert(!result.ok());

    // The bridge is intentionally backend-neutral until OCCT is linked.
    assert(result.error == mir::io::step::OcctBridge::availabilityMessage() ||
           !mir::io::step::OcctBridge::available());

    return 0;
}
