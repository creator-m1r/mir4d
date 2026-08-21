#pragma once

#include "Event.hpp"
#include "../Object/ObjectID.hpp"

namespace MirUI
{

struct TransformChangedEvent
{
    ObjectID objectId{};
    Event source{};
};

}
