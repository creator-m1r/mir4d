// MirUI/Core/Rendering/RenderCommand.hpp
// Команды отрисовки — описывают, ЧТО нужно нарисовать, но не КАК.
// Это универсальные графические примитивы, понятные любому рендереру.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Layout/Rect.hpp"
#include "../Layout/Point.hpp"
#include "../../Foundation/Color/Color.hpp"
#include <string>
#include <variant>

namespace MirUI {

// Типы команд отрисовки — простые, как кубики конструктора.
enum class RenderCommandType {
    DrawRect,       // Нарисовать прямоугольник
    DrawText,       // Нарисовать текст
    DrawLine,       // Нарисовать линию
    DrawImage,      // Нарисовать картинку
    DrawPath,       // Нарисовать сложный путь (контур)
    Clip,           // Обрезать всё, что выходит за границы
    PushTransform,  // Сохранить текущее положение и применить смещение/поворот
    PopTransform    // Вернуть сохранённое положение
};

// Одна команда отрисовки — как письмо художнику.
struct RenderCommand {
    RenderCommandType type; // Что именно делать

    // Данные, которые могут понадобиться для разных команд.
    // Используем std::variant, чтобы хранить только нужное в данный момент.
    std::variant<
        Rect,           // для DrawRect, Clip
        std::string,    // для DrawText
        Point,          // для DrawLine (конечная точка, начальная — из состояния)
        std::pair<Point, Point> // для DrawLine (начало и конец)
    > data;

    // Цвет и толщина линии (для соответствующих команд).
    Color fillColor   = Color::transparent(); // цвет заливки
    Color strokeColor = Color::transparent(); // цвет рамки
    double strokeWidth = 1.0;                 // толщина рамки

    // Для текста: сам текст, шрифт и размер уже в data (строка),
    // а эти поля — для дополнительных настроек (в будущем).
    // Пока оставим простой вариант.
};

} // namespace MirUI