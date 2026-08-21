#pragma once

// Public top-level API for the M1R engineering engine.
//
// Consumers should normally include this file instead of reaching into
// implementation subdirectories. Domain-specific umbrella headers remain
// available for smaller compile surfaces.

#include "Core/Core.hpp"
#include "Geometry/Geometry.hpp"
#include "BRep/BRep.hpp"
#include "Viewport/Viewport.hpp"
#include "Interaction/Interaction.hpp"
