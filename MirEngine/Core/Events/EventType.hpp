
#pragma once

#include <cstdint>

namespace mir {

enum class EventType : uint32_t {

    DocumentCreated,
    DocumentOpened,
    DocumentClosed,
    DocumentModified,

    ObjectAdded,
    ObjectRemoved,
    ObjectChanged,

    EntityCreated,
    EntityDestroyed,
    EntityMoved,
    EntityTransformed,

    SceneChanged,
    SceneCleared,

    SelectionChanged,

    FeatureAdded,
    FeatureRemoved,
    FeatureModified,

    ToolChanged,

    ViewChanged,
    CameraChanged,

    EngineInitialized,
    EngineShutdown,

    Unknown
};

}