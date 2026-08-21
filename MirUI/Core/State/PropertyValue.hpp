
#pragma once

#include <variant>
#include <string>
#include <cstdint>

#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include "../Layout/Point.hpp"
#include "../Layout/Size.hpp"
#include "../Layout/Rect.hpp"
#include "../Layout/Insets.hpp"

namespace MirUI {

using PropertyValue = std::variant<
    std::monostate,
    bool,
    int64_t,
    double,
    std::string,
    Color,
    Font,
    Point,
    Size,
    Rect,
    Insets
>;

}