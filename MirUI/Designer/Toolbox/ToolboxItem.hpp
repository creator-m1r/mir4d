// MirUI/Designer/Toolbox/ToolboxItem.hpp
// 🧩 Элемент панели инструментов (Toolbox) — описывает один тип виджета,
// доступный для создания в MirUI Designer.
//
// Каждый элемент содержит:
//   • id         — уникальный строковый идентификатор (например, "Button")
//   • name       — отображаемое имя (например, "Кнопка")
//   • widgetType — тип виджета из WidgetType (Button, Label, Tree…)
//   • icon       — иконка для отображения в тулбоксе (платформонезависимая)
//   • tooltip    — всплывающая подсказка с описанием
//
// Структура используется ToolboxModel для построения списка доступных виджетов.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include "../../Core/Widget/WidgetType.hpp"
#include "../../Foundation/Icons/IconID.hpp"

namespace MirUI {

struct ToolboxItem {
    std::string id;           // уникальный идентификатор элемента (например, "button")
    std::string name;         // отображаемое имя (например, "Кнопка")
    WidgetType  widgetType;   // тип создаваемого виджета
    IconID      icon;         // иконка для панели инструментов (строка-идентификатор)
    std::string tooltip;      // описание, которое появляется при наведении
};

} // namespace MirUI