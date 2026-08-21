#pragma once

#include "ObjectID.hpp"
#include "ObjectType.hpp"

#include <string>

namespace MirUI
{

struct ObjectProperties
{
    ObjectID id = mir::InvalidObjectId;
    ObjectType type = ObjectType::Unknown;
    std::string name{};
    std::string category{};
    bool visible = true;
    bool selectable = true;
    bool locked = false;

    [[nodiscard]] bool valid() const noexcept
    {
        return isValidObjectID(id);
    }
};

} // namespace MirUI
