
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

}