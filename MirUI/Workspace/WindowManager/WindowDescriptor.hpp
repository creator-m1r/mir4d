
#pragma once

#include <string>
#include "../../Core/Layout/Size.hpp"
#include "../../Core/Layout/Point.hpp"

namespace MirUI {

struct WindowDescriptor {

    std::string id;

    std::string title;

    Size size = { 1280.0, 720.0 };

    Point position = { 100.0, 100.0 };

    bool resizable = true;

    bool maximized = false;
};

}