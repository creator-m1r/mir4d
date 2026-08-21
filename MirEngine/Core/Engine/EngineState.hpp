
#pragma once

namespace mir {

enum class EngineState {
    Created,
    Initializing,
    Ready,
    Running,
    ShuttingDown,
    Shutdown
};

}