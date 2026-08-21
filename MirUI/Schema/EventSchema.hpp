// MirUI/Schema/EventSchema.hpp
// 📡 Схема событий MirUI — реестр всех стандартных событий системы.
//
// Аналогично CommandSchema, EventSchema определяет полный перечень
// встроенных типов событий (MouseDown, KeyUp, FocusGained, Click…),
// их категории для группировки, человекочитаемые имена, описания и
// типовые источники. Это служит контрактом между ядром и платформенными
// адаптерами: рендереры (SwiftUI, WinUI, WebUI) могут читать схему
// и автоматически настраивать маршрутизацию системных событий
// в универсальные события MirUI.
//
// Кроме того, Designer использует схему для отображения списка событий,
// которые можно привязать к виджету (например, «при щелчке»), и генерации
// кода/конфигурации.
//
// Структура EventDescriptor содержит:
//   • type        — EventType (из перечисления Core/Events/EventType.hpp)
//   • category    — категория (например, "Мышь", "Клавиатура", "Фокус")
//   • name        — короткое имя на русском (например, "Щелчок мыши")
//   • description — подробное описание (тултип)
//   • source      — тип источника: "mouse", "keyboard", "focus", "drag", "layout"
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Core/Events/EventType.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace MirUI {

// ── Дескриптор одного события ──────────────────────────────
struct EventDescriptor {
    EventType   type;         // тип события из перечисления
    std::string category;     // категория для группировки ("Мышь", "Клавиатура", …)
    std::string name;         // отображаемое имя ("Щелчок мыши")
    std::string description;  // подробное описание
    std::string source;       // источник: "mouse", "keyboard", "focus", "drag", "layout", "system"
};

// ── Схема (реестр) событий ─────────────────────────────────
class EventSchema {
public:
    // Получить полный список всех зарегистрированных событий.
    [[nodiscard]] static const std::vector<EventDescriptor>& allEvents() {
        return events();
    }

    // Найти событие по его типу. Возвращает указатель на дескриптор или nullptr.
    [[nodiscard]] static const EventDescriptor* find(EventType type) {
        auto& evts = events();
        auto it = std::find_if(evts.begin(), evts.end(),
            [type](const EventDescriptor& desc) { return desc.type == type; });
        return (it != evts.end()) ? &(*it) : nullptr;
    }

    // Найти все события из заданной категории.
    [[nodiscard]] static std::vector<const EventDescriptor*> findByCategory(const std::string& category) {
        std::vector<const EventDescriptor*> result;
        for (const auto& desc : events()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

    // Получить список всех уникальных категорий событий.
    [[nodiscard]] static std::vector<std::string> allCategories() {
        std::vector<std::string> categories;
        for (const auto& desc : events()) {
            if (std::find(categories.begin(), categories.end(), desc.category) == categories.end()) {
                categories.push_back(desc.category);
            }
        }
        return categories;
    }

private:
    // Статический реестр всех встроенных событий.
    static std::vector<EventDescriptor>& events() {
        static std::vector<EventDescriptor> s_events = {
            // ── Мышь ─────────────────────────────────────────
            {
                EventType::MouseMove,
                "Мышь",
                "Движение мыши",
                "Курсор перемещается над виджетом",
                "mouse"
            },
            {
                EventType::MouseDown,
                "Мышь",
                "Нажатие кнопки мыши",
                "Кнопка мыши нажата над виджетом",
                "mouse"
            },
            {
                EventType::MouseUp,
                "Мышь",
                "Отпускание кнопки мыши",
                "Кнопка мыши отпущена над виджетом",
                "mouse"
            },
            {
                EventType::MouseWheel,
                "Мышь",
                "Колёсико мыши",
                "Прокрутка колёсика мыши",
                "mouse"
            },
            {
                EventType::Click,
                "Мышь",
                "Щелчок мыши",
                "Быстрое нажатие и отпускание левой кнопки",
                "mouse"
            },
            {
                EventType::DoubleClick,
                "Мышь",
                "Двойной щелчок",
                "Два быстрых щелчка левой кнопкой",
                "mouse"
            },

            // ── Перетаскивание ───────────────────────────────
            {
                EventType::DragBegin,
                "Перетаскивание",
                "Начало перетаскивания",
                "Пользователь начал перетаскивать объект",
                "drag"
            },
            {
                EventType::DragMove,
                "Перетаскивание",
                "Перетаскивание",
                "Объект перемещается за курсором",
                "drag"
            },
            {
                EventType::DragEnd,
                "Перетаскивание",
                "Завершение перетаскивания",
                "Пользователь отпустил перетаскиваемый объект",
                "drag"
            },

            // ── Клавиатура ───────────────────────────────────
            {
                EventType::KeyDown,
                "Клавиатура",
                "Нажатие клавиши",
                "Клавиша на клавиатуре нажата",
                "keyboard"
            },
            {
                EventType::KeyUp,
                "Клавиатура",
                "Отпускание клавиши",
                "Клавиша на клавиатуре отпущена",
                "keyboard"
            },

            // ── Фокус ────────────────────────────────────────
            {
                EventType::FocusGained,
                "Фокус",
                "Получение фокуса",
                "Виджет стал активным (получил фокус ввода)",
                "focus"
            },
            {
                EventType::FocusLost,
                "Фокус",
                "Потеря фокуса",
                "Виджет перестал быть активным (потерял фокус)",
                "focus"
            },

            // ── Изменение размера и компоновки ───────────────
            {
                EventType::Resize,
                "Размер",
                "Изменение размера",
                "Виджет изменил свои размеры",
                "layout"
            },
            {
                EventType::LayoutChanged,
                "Размер",
                "Изменение компоновки",
                "Дерево виджетов было перестроено (layout)",
                "layout"
            }
        };
        return s_events;
    }
};

} // namespace MirUI