// MirUI/Widgets/PropertyGrid/Property.hpp
// Структура, описывающая одно свойство для панели свойств (PropertyGrid).
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include <variant>
#include <cstdint>
#include "../../Core/State/StateValue.hpp"  // используем тот же тип значения, что и в StateStore

namespace MirUI {

// Тип значения свойства — тот же универсальный вариант,
// который уже определён в Core/State/StateValue.hpp:
//   using StateValue = std::variant<bool, int64_t, double, std::string>;

// Структура, представляющая одно редактируемое свойство в инспекторе.
struct Property {
    std::string id;          // Уникальный идентификатор свойства (например, "object.name").
    std::string name;        // Отображаемое имя свойства (например, "Имя объекта").
    std::string category;    // Группа, в которой будет показано свойство (например, "Transform").

    StateValue value;        // Текущее значение свойства. Используем тот же тип, что и везде в MirUI.
    
    bool readOnly = false;   // Если true, пользователь не может изменить значение через PropertyGrid.
};

} // namespace MirUI