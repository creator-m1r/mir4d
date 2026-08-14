#pragma once

#include "../ImportOptions.hpp"

#include <string>

namespace mir::io::step
{

/// Backend-neutral seam for OpenCASCADE integration.
/// OCCT types intentionally remain private to the implementation layer.
class OcctBridge
{
public:
    [[nodiscard]] static bool available() noexcept;

    /// STEP/IGES exact B-Rep import is intentionally kept behind this boundary.
    /// The current implementation reports unavailable when MIR_ENABLE_OCCT is off.
    [[nodiscard]] static std::string availabilityMessage();
};

} // namespace mir::io::step
