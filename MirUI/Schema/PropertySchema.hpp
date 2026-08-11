// MirUI/Schema/PropertySchema.hpp
// 🧾 Схема свойств MirUI — центральный реестр всех стандартных свойств виджетов.
//
// Когда инспектор свойств (PropertyGrid / InspectorModel) сталкивается с
// незнакомым виджетом, он не знает, какие именно свойства у этого виджета есть
// и как их редактировать. PropertySchema решает эту проблему:
// здесь в одном месте описаны ВСЕ встроенные свойства, которые могут встречаться
// в любых виджетах MirUI: их идентификаторы, типы данных, категории, названия,
// значения по умолчанию и возможные значения для перечислений.
//
// Благодаря этому:
//   • InspectorModel может автоматически построить список свойств для любого
//     виджета, просто сопоставив фактические свойства виджета с записями в схеме.
//   • PropertyEditor понимает, какой тип редактора (строка, число, цвет, шрифт,
//     перечисление) нужно показать для каждого свойства.
//   • Рендереры (SwiftUI, WinUI) могут использовать схему для документации и
//     автоматической генерации кода.
//   • Дизайнер (Designer) может показывать осмысленные названия на русском языке
//     вместо технических идентификаторов.
//
// Каждая запись (PropertyDescriptor) содержит:
//   • id          — строковый идентификатор свойства (например, "text", "width")
//   • type        — ожидаемый тип значения (String, Integer, Float, Boolean, Color, Font, Enum…)
//   • category    — категория для группировки в инспекторе ("Основные", "Геометрия", "Стиль")
//   • name        — короткое имя на русском (например, "Текст", "Ширина")
//   • description — подробное описание (тултип)
//   • defaultValue — значение по умолчанию (если свойство не задано у виджета)
//   • enumValues  — список допустимых значений, если тип — Enum (пустой для остальных)
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include "../Core/State/StateValue.hpp"

namespace MirUI {

// ── Типы данных, используемых в свойствах ──────────────────
enum class PropertyValueType {
    String,
    Integer,
    Float,
    Boolean,
    Color,      // хранится как строка "#RRGGBBAA"
    Font,       // хранится как строка "family;size;weight;style"
    Enum,       // строковое значение из фиксированного списка
    Size,       // в будущем: составной тип (ширина, высота, единицы)
    Rect        // в будущем: составной тип (x, y, width, height)
};

// ── Дескриптор одного стандартного свойства ────────────────
struct PropertyDescriptor {
    std::string id;              // уникальный идентификатор (например, "text", "width")
    PropertyValueType type;      // ожидаемый тип значения
    std::string category;        // категория ("Основные", "Геометрия", "Стиль", "Поведение")
    std::string name;            // отображаемое имя на русском ("Текст", "Ширина")
    std::string description;     // подробное описание для тултипа
    StateValue defaultValue;     // значение по умолчанию (если не задано)
    std::vector<std::string> enumValues; // возможные значения для Enum (пустой для других типов)
};

// ── Схема (реестр) свойств ─────────────────────────────────
class PropertySchema {
public:
    // Получить полный список всех зарегистрированных свойств.
    [[nodiscard]] static const std::vector<PropertyDescriptor>& allProperties() {
        return properties();
    }

    // Найти дескриптор свойства по его идентификатору.
    // Возвращает указатель на дескриптор или nullptr, если свойство не найдено.
    [[nodiscard]] static const PropertyDescriptor* find(const std::string& id) {
        auto& props = properties();
        auto it = std::find_if(props.begin(), props.end(),
            [&id](const PropertyDescriptor& desc) { return desc.id == id; });
        return (it != props.end()) ? &(*it) : nullptr;
    }

    // Найти все свойства из заданной категории.
    [[nodiscard]] static std::vector<const PropertyDescriptor*> findByCategory(const std::string& category) {
        std::vector<const PropertyDescriptor*> result;
        for (const auto& desc : properties()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

    // Получить список всех уникальных категорий свойств.
    [[nodiscard]] static std::vector<std::string> allCategories() {
        std::vector<std::string> categories;
        for (const auto& desc : properties()) {
            if (std::find(categories.begin(), categories.end(), desc.category) == categories.end()) {
                categories.push_back(desc.category);
            }
        }
        return categories;
    }

    // Получить значение по умолчанию для заданного свойства.
    [[nodiscard]] static std::optional<StateValue> defaultValue(const std::string& id) {
        auto desc = find(id);
        if (desc) {
            return desc->defaultValue;
        }
        return std::nullopt;
    }

private:
    // Статический реестр всех встроенных свойств.
    static std::vector<PropertyDescriptor>& properties() {
        static std::vector<PropertyDescriptor> s_properties = {
            // ── Основные ──────────────────────────────────────
            {
                "name",
                PropertyValueType::String,
                "Основные",
                "Имя",
                "Уникальное имя виджета, используемое для идентификации в коде",
                StateValue(std::string("")),
                {}
            },
            {
                "visible",
                PropertyValueType::Boolean,
                "Основные",
                "Видимость",
                "Показывать ли виджет на экране",
                StateValue(true),
                {}
            },
            {
                "enabled",
                PropertyValueType::Boolean,
                "Основные",
                "Доступность",
                "Может ли пользователь взаимодействовать с виджетом",
                StateValue(true),
                {}
            },
            {
                "text",
                PropertyValueType::String,
                "Основные",
                "Текст",
                "Текст, отображаемый на виджете (например, на кнопке или надписи)",
                StateValue(std::string("")),
                {}
            },

            // ── Геометрия ─────────────────────────────────────
            {
                "width",
                PropertyValueType::Float,
                "Геометрия",
                "Ширина",
                "Ширина виджета в пикселях",
                StateValue(0.0),
                {}
            },
            {
                "height",
                PropertyValueType::Float,
                "Геометрия",
                "Высота",
                "Высота виджета в пикселях",
                StateValue(0.0),
                {}
            },
            {
                "minWidth",
                PropertyValueType::Float,
                "Геометрия",
                "Мин. ширина",
                "Минимально допустимая ширина виджета",
                StateValue(0.0),
                {}
            },
            {
                "minHeight",
                PropertyValueType::Float,
                "Геометрия",
                "Мин. высота",
                "Минимально допустимая высота виджета",
                StateValue(0.0),
                {}
            },
            {
                "maxWidth",
                PropertyValueType::Float,
                "Геометрия",
                "Макс. ширина",
                "Максимально допустимая ширина виджета",
                StateValue(1e9),
                {}
            },
            {
                "maxHeight",
                PropertyValueType::Float,
                "Геометрия",
                "Макс. высота",
                "Максимально допустимая высота виджета",
                StateValue(1e9),
                {}
            },
            {
                "alignment",
                PropertyValueType::Enum,
                "Геометрия",
                "Выравнивание",
                "Способ выравнивания текста или содержимого внутри виджета",
                StateValue(std::string("Left")),
                {"Left", "Center", "Right"}
            },

            // ── Стиль ─────────────────────────────────────────
            {
                "color",
                PropertyValueType::Color,
                "Стиль",
                "Цвет фона",
                "Основной цвет фона виджета",
                StateValue(std::string("#FFFFFF")),
                {}
            },
            {
                "font",
                PropertyValueType::Font,
                "Стиль",
                "Шрифт",
                "Шрифт, используемый для отображения текста",
                StateValue(std::string("System;14;400;0")), // family;size;weight;style
                {}
            },
            {
                "cornerRadius",
                PropertyValueType::Float,
                "Стиль",
                "Радиус скругления",
                "Радиус закругления углов виджета (0 — без скругления)",
                StateValue(0.0),
                {}
            },
            {
                "opacity",
                PropertyValueType::Float,
                "Стиль",
                "Непрозрачность",
                "Степень прозрачности виджета (от 0.0 — полностью прозрачный до 1.0 — непрозрачный)",
                StateValue(1.0),
                {}
            },
            {
                "borderWidth",
                PropertyValueType::Float,
                "Стиль",
                "Толщина границы",
                "Толщина рамки вокруг виджета в пикселях",
                StateValue(0.0),
                {}
            },
            {
                "borderColor",
                PropertyValueType::Color,
                "Стиль",
                "Цвет границы",
                "Цвет рамки виджета",
                StateValue(std::string("#000000")),
                {}
            },

            // ── Поведение ─────────────────────────────────────
            {
                "command",
                PropertyValueType::String,
                "Поведение",
                "Команда",
                "Идентификатор команды (CommandID), выполняемой при нажатии на кнопку",
                StateValue(std::string("")),
                {}
            },
            {
                "checked",
                PropertyValueType::Boolean,
                "Поведение",
                "Нажатое состояние",
                "Находится ли кнопка-переключатель в нажатом состоянии",
                StateValue(false),
                {}
            }
        };
        return s_properties;
    }
};

} // namespace MirUI