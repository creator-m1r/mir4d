
#pragma once

#include <string>

namespace mir {

struct EngineConfig {

    bool enableLogging = true;

    bool enableValidation = true;

    int logLevel = 2;

    std::string applicationName = "MirEngine";

    static constexpr int LogLevelDebug    = 1;
    static constexpr int LogLevelInfo     = 2;
    static constexpr int LogLevelWarning  = 3;
    static constexpr int LogLevelError    = 4;
    static constexpr int LogLevelCritical = 5;
};

}