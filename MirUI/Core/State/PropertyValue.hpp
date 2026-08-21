// MirUI/Core/State/PropertyValue.hpp
// 🧾 Расширенный универсальный тип значения свойства виджета.
//
// В отличие от простого StateValue (который содержит только базовые типы:
// bool, int64_t, double, std::string), PropertyValue дополнительно включает
// геометрические и стилевые структуры — Color, Font, Point, Size, Rect, Insets.
// Это позволяет инспектору свойств работать напрямую с понятными типами,
// а не сериализованными строками для каждого цвета или шрифта.
//
// Используется в:
//   • PropertyDescriptor — описание свойства (тип, значение по умолчанию)
//   • Widget::getProperty / setProperty — доступ к свойствам виджета
//   • ChangePropertyCommand — сохранение старого и нового значений для Undo/Redo
//   • InspectorModel — отображение и редактирование свойств выделенного виджета
//
// Включает:
//   • std::monostate     — отсутствующее / не заданное значение
//   • bool               — логическое свойство (видимость, доступность, галочка)
//   • int64_t            — целое число (id, счётчики)
//   • double             — дробное число (ширина, высота, радиус, отступ)
//   • std::string        — произвольный текст (имя, заголовок, команда)
//   • Color              — цвет в формате RGBA (из Foundation/Color/Color.hpp)
//   • Font               — описание шрифта (семейство, размер, вес, стиль)
//   • Point              — координаты точки (x, y)
//   • Size               — размеры (ширина, высота)
//   • Rect               — прямоугольник (x, y, width, height)
//   • Insets             — отступы (top, right, bottom, left)
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <variant>
#include <string>
#include <cstdint>

// Подключаем необходимые типы из Foundation и Core/Layout.
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include "../Layout/Point.hpp"
#include "../Layout/Size.hpp"
#include "../Layout/Rect.hpp"
#include "../Layout/Insets.hpp"

namespace MirUI {

// Сам тип — просто синоним для std::variant с перечисленными выше типами.
// Теперь любой код, который работает со свойствами, может обрабатывать
// любое значение универсальным образом (через std::visit).
using PropertyValue = std::variant<
    std::monostate,   // отсутствие значения
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

} // namespace MirUI