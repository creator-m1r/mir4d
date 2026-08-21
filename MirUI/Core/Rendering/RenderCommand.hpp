
#pragma once

#include "../Layout/Rect.hpp"
#include "../Layout/Point.hpp"
#include "../../Foundation/Color/Color.hpp"
#include <string>
#include <variant>

namespace MirUI {

enum class RenderCommandType {
    DrawRect,
    DrawText,
    DrawLine,
    DrawImage,
    DrawPath,
    Clip,
    PushTransform,
    PopTransform
};

struct RenderCommand {
    RenderCommandType type;

    std::variant<
        Rect,
        std::string,
        Point,
        std::pair<Point, Point>
    > data;

    Color fillColor   = Color::transparent();
    Color strokeColor = Color::transparent();
    double strokeWidth = 1.0;

};

}