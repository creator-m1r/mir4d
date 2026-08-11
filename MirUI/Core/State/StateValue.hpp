// MirUI/Core/State/StateValue.hpp
// Universal state value type – currently supports bool, int64_t, double, std::string.
// Pure C++23, no platform dependencies.

#pragma once

#include <variant>
#include <string>
#include <cstdint>

namespace MirUI {

using StateValue = std::variant<
    bool,
    int64_t,
    double,
    std::string
>;

} // namespace MirUI