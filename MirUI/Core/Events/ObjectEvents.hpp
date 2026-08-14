#pragma once

#include "Event.hpp"
#include "../Object/ObjectID.hpp"
#include "../Object/ObjectType.hpp"

namespace MirUI
{

struct ObjectCreatedEvent : Event
{
    ObjectID objectId{};
    ObjectType objectType = ObjectType::Unknown;

    ObjectCreatedEvent(ObjectID id, ObjectType type)
        : Event(EventType::ObjectCreated), objectId(id), objectType(type)
    {
        data0 = id.value();
    }
};

struct ObjectDeletedEvent : Event
{
    ObjectID objectId{};
    ObjectType objectType = ObjectType::Unknown;

    ObjectDeletedEvent(ObjectID id, ObjectType type)
        : Event(EventType::ObjectDeleted), objectId(id), objectType(type)
    {
        data0 = id.value();
    }
};

struct ObjectModifiedEvent : Event
{
    ObjectID objectId{};
    ObjectType objectType = ObjectType::Unknown;

    ObjectModifiedEvent(ObjectID id, ObjectType type)
        : Event(EventType::ObjectModified), objectId(id), objectType(type)
    {
        data0 = id.value();
    }
};

} // namespace MirUI
