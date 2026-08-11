// MirUI/Designer/Inspector/PropertyEditorFactory.hpp
// 🏭 Фабрика редакторов свойств — создаёт нужный редактор по типу свойства.
//
// Когда инспектор свойств (InspectorModel) получает список PropertyEntry
// для выделенного виджета, каждая запись содержит PropertyEditorType —
// тип редактора, который нужно показать: String, Integer, Float, Boolean,
// Color, Font или Enum. PropertyEditorFactory — это центральное место,
// которое по этому типу создаёт соответствующий объект PropertyEditor.
//
// Благодаря фабрике код инспектора не зависит от конкретных классов редакторов.
// Ему достаточно вызвать PropertyEditorFactory::create(entry), и он получит
// готовый редактор, который можно отобразить на экране.
//
// Сама фабрика не хранит состояние и не занимается отрисовкой —
// это просто удобная утилита для создания объектов.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "PropertyEditor.hpp"    // базовый редактор (содержит внутри ColorEditor, FontEditor, EnumEditor)
#include "InspectorModel.hpp"    // PropertyEntry, PropertyEditorType
#include "../Document/UIDocument.hpp"
#include <memory>

namespace MirUI {

class PropertyEditorFactory {
public:
    // ── Главный метод создания ──────────────────────────────
    // Принимает:
    //   doc      — документ, к которому принадлежит редактируемый виджет
    //   widgetId — идентификатор виджета, чьё свойство редактируем
    //   entry    — описание свойства (имя, тип, текущее значение, возможные enum-значения)
    //
    // Возвращает умный указатель на конкретный PropertyEditor, готовый к использованию.
    // Сам PropertyEditor уже содержит внутри себя специализированные редакторы
    // (ColorEditor, FontEditor, EnumEditor), если это необходимо.
    static std::unique_ptr<PropertyEditor> create(UIDocument& doc,
                                                  WidgetID widgetId,
                                                  const PropertyEntry& entry) {
        // PropertyEditor сам разберётся, какой внутренний редактор создать,
        // в зависимости от entry.editorType.
        return std::make_unique<PropertyEditor>(doc, widgetId, entry);
    }
};

} // namespace MirUI