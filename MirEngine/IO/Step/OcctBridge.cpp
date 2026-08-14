#include "OcctBridge.hpp"

namespace mir::io::step
{

bool OcctBridge::available() noexcept
{
#if defined(MIR_ENABLE_OCCT) && MIR_ENABLE_OCCT
    return true;
#else
    return false;
#endif
}

std::string OcctBridge::availabilityMessage()
{
    if (available())
        return "OCCT bridge is enabled; STEP/IGES implementation can be linked here.";

    return "OCCT bridge is disabled. Configure with MIR_ENABLE_OCCT and link OpenCASCADE before enabling STEP/IGES IO.";
}

} // namespace mir::io::step
