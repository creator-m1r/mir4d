// MirUI/Workspace/WindowManager/WindowDescriptor.hpp
// Описание окна: его идентификатор, заголовок, начальный размер и позиция,
// а также флаги: можно ли изменять размер и развёрнуто ли окно на весь экран.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include "../../Core/Layout/Size.hpp"
#include "../../Core/Layout/Point.hpp"

namespace MirUI {

struct WindowDescriptor {
    // Уникальный идентификатор окна (например, "main", "settings").
    std::string id;

    // Заголовок окна, который отображается в строке заголовка.
    std::string title;

    // Начальный размер окна (в логических пикселях).
    Size size = { 1280.0, 720.0 };

    // Начальная позиция левого верхнего угла окна на экране.
    Point position = { 100.0, 100.0 };

    // Можно ли изменять размер окна (если false, то окно фиксированного размера).
    bool resizable = true;

    // Развёрнуто ли окно на весь экран при запуске.
    bool maximized = false;
};

} // namespace MirUI