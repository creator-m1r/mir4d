#pragma once

#include "MirEngine/Math/Transform.hpp"
#include "MirEngine/Core/Identity/ObjectId.hpp"

namespace MirUI
{

// Inspector-facing view of the canonical MirEngine transform.
// MirUI deliberately does not define a second Transform type.
struct TransformProperties
{
    mir4d::ObjectId objectId = mir4d::InvalidObjectId;
    mir::Transform transform{};
};

} // namespace MirUI
