// MirUI/Core/Widget/WidgetSnapshot.hpp
// 📸 Снимок виджета — безопасное, независимое от платформы представление
//    состояния любого виджета для передачи рендерерам.
//
// Рендереры (SwiftUI, WinUI, WebUI) не должны работать с живыми C++ объектами
// Widget напрямую — это нарушает инкапсуляцию и усложняет межъязыковое
// взаимодействие. Вместо этого UIRuntime создаёт WidgetSnapshot —
// легковесную структуру, которая содержит все публичные свойства виджета
// в виде простых данных (строки, числа, булевы значения).
//
// WidgetSnapshot можно:
//   • Сериализовать в JSON / бинарный формат и передать через C-ABI.
//   • Преобразовать в Swift-структуру или C#-класс.
//   • Сравнивать с предыдущим снимком для оптимизации обновлений (diff).
//
// Структура включает:
//   • id, type           — идентификатор и тип виджета.
//   • name               — человекочитаемое имя.
//   • visible, enabled   — базовые флаги состояния.
//   • bounds             — позиция и размер (Rect).
//   • properties         — карта всех пользовательских свойств (ключ → строковое значение).
//   • children           — рекурсивный список дочерних снимков.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "WidgetType.hpp"            // WidgetType
#include "../Layout/Rect.hpp"        // Rect
#include <string>
#include <vector>
#include <unordered_map>

namespace MirUI {

// Временное определение WidgetID до полной миграции неймспейса.
using WidgetID = std::string;

struct WidgetSnapshot {
    // ── Идентификация ────────────────────────────────────────
    WidgetID   id;                          // строковый идентификатор
    WidgetType type = WidgetType::Unknown;  // тип виджета
    std::string name;                       // человекочитаемое имя

    // ── Базовые флаги ────────────────────────────────────────
    bool visible = true;
    bool enabled = true;

    // ── Геометрия ────────────────────────────────────────────
    Rect bounds;                             // положение и размер

    // ── Пользовательские свойства ────────────────────────────
    std::unordered_map<std::string, std::string> properties;

    // ── Дочерние виджеты ─────────────────────────────────────
    std::vector<WidgetSnapshot> children;

    // ── Операторы сравнения (явная реализация) ──────────────
    bool operator==(const WidgetSnapshot& other) const {
        return id == other.id &&
               type == other.type &&
               name == other.name &&
               visible == other.visible &&
               enabled == other.enabled &&
               bounds == other.bounds &&
               properties == other.properties &&
               children == other.children;
    }
    bool operator!=(const WidgetSnapshot& other) const {
        return !(*this == other);
    }
};

} // namespace MirUI