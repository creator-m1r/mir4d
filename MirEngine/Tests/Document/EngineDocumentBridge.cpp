#include "../../Document/CreateBoxCommandHandler.hpp"
#include "../../Document/Document.hpp"
#include "../../../MirUI/Interop/MIR4DEngineDocumentBridge.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

int main()
{
    MirUI::MIR4DEngineDocumentBridge bridge;
    bridge.reset("Bridge Test");

    assert(std::string(bridge.projectName()) == "Bridge Test");
    assert(bridge.objectCount() == 0);
    assert(bridge.commandCount() == 0);
    assert(bridge.currentTime() == 0.0);
    assert(bridge.isValid());

    std::uint64_t objectId = mir4d::InvalidObjectId;
    assert(bridge.createBox(100.0, 60.0, 40.0, &objectId));
    assert(mir4d::isValidObjectId(objectId));
    assert(bridge.objectCount() == 1);
    assert(bridge.commandCount() == 1);
    assert(bridge.meshVertexCount(objectId) >= 8);
    assert(bridge.meshTriangleCount(objectId) >= 12);
    assert(bridge.isModified());
    assert(bridge.isValid());

    assert(bridge.advanceTime(2.5));
    assert(bridge.currentTime() == 2.5);

    std::cout << "MIR4D ENGINE DOCUMENT BRIDGE: OK\n";
    return 0;
}
