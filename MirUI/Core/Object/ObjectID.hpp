#pragma once

#include "MirEngine/Core/Identity/ObjectId.hpp"

namespace MirUI
{

// UI deliberately reuses the engine identity type.
// There is no second ObjectID implementation in MirUI.
using ObjectID = mir4d::ObjectId;

[[nodiscard]] constexpr bool isValidObjectID(ObjectID id) noexcept
{
    return mir4d::isValidObjectId(id);
}

} // namespace MirUI
