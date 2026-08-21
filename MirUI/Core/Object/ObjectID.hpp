#pragma once

#include "MirEngine/Core/Identity/ObjectId.hpp"

namespace MirUI
{

using ObjectID = mir4d::ObjectId;

[[nodiscard]] constexpr bool isValidObjectID(ObjectID id) noexcept
{
    return mir4d::isValidObjectId(id);
}

}
