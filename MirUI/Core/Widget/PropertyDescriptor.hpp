// MirUI/Core/Widget/PropertyDescriptor.hpp
// 📋 Дескриптор свойства виджета — описание одного свойства для инспектора и редактора.
//
// Когда мы смотрим на кнопку в инспекторе, мы видим не просто значение,
// а целую строчку с названием, типом и, возможно, подсказкой.
// PropertyDescriptor хранит всю эту информацию в одном месте,
// чтобы InspectorModel могла автоматически строить интерфейс,
// а PropertyEditor знал, какой редактор показывать.
//
// Каждый виджет (Button, Label, Tree…) может объявить статический список
// своих свойств — массив PropertyDescriptor. Инспектор читает этот список
// и создаёт нужные редакторы: для строки — поле ввода, для цвета — палитру,
// для шрифта — диалог выбора шрифта, для перечисления — выпадающий список.
//
// Поля:
//   • id           — строковый ключ свойства (например, "text", "color").
//                    Именно под этим именем свойство хранится в Widget::m_properties.
//   • displayName  — человекочитаемое название на русском («Текст», «Цвет фона»).
//   • type         — тип значения (см. PropertyType.hpp): String, Color, Font, Enum и т.д.
//   • defaultValue — значение по умолчанию, которое будет установлено при создании виджета.
//   • editable     — можно ли редактировать это свойство (если false, поле заблокировано).
//   • visible      — показывать ли свойство в инспекторе (можно скрыть технические поля).
//   • enumValues   — для типа Enum список допустимых значений (для остальных типов — пустой).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "PropertyType.hpp"
#include "../State/PropertyValue.hpp"  // пока это StateValue, в будущем будет расширен
#include <string>
#include <vector>
#include <optional>

namespace MirUI {

struct PropertyDescriptor {
    std::string id;                  // строковый ключ (например, "text", "width")
    std::string displayName;         // название для пользователя (например, "Текст", "Ширина")
    PropertyType type;               // тип значения (String, Integer, Number, Boolean, Color, Font, Enum...)
    PropertyValue defaultValue;      // значение по умолчанию

    bool editable = true;            // можно ли редактировать
    bool visible  = true;            // показывать ли в инспекторе

    // Список допустимых значений, если type == PropertyType::Enum.
    // Для остальных типов оставляем пустым.
    std::vector<std::string> enumValues;

    // ── Конструкторы для удобства ────────────────────────────
    PropertyDescriptor() = default;

    // Простой конструктор для свойств, которые всегда редактируемы и видны.
    PropertyDescriptor(std::string id,
                       std::string displayName,
                       PropertyType type,
                       PropertyValue defaultValue,
                       std::vector<std::string> enumVals = {})
        : id(std::move(id))
        , displayName(std::move(displayName))
        , type(type)
        , defaultValue(std::move(defaultValue))
        , editable(true)
        , visible(true)
        , enumValues(std::move(enumVals))
    {}

    // Полный конструктор с указанием editable/visible.
    PropertyDescriptor(std::string id,
                       std::string displayName,
                       PropertyType type,
                       PropertyValue defaultValue,
                       bool editable,
                       bool visible,
                       std::vector<std::string> enumVals = {})
        : id(std::move(id))
        , displayName(std::move(displayName))
        , type(type)
        , defaultValue(std::move(defaultValue))
        , editable(editable)
        , visible(visible)
        , enumValues(std::move(enumVals))
    {}
};

} // namespace MirUI