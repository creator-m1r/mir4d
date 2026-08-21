#pragma once

#include "MirEngine/Math/Transform.hpp"
#include "MirEngine/Core/Identity/ObjectId.hpp"

namespace MirUI
{

struct TransformProperties
{
    mir4d::ObjectId objectId = mir4d::InvalidObjectId;
    mir::Transform transform{};
};

}
