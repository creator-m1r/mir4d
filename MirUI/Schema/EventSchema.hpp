
#pragma once

#include "../Core/Events/EventType.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace MirUI {

struct EventDescriptor {
    EventType   type;
    std::string category;
    std::string name;
    std::string description;
    std::string source;
};

class EventSchema {
public:

    [[nodiscard]] static const std::vector<EventDescriptor>& allEvents() {
        return events();
    }

    [[nodiscard]] static const EventDescriptor* find(EventType type) {
        auto& evts = events();
        auto it = std::find_if(evts.begin(), evts.end(),
            [type](const EventDescriptor& desc) { return desc.type == type; });
        return (it != evts.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static std::vector<const EventDescriptor*> findByCategory(const std::string& category) {
        std::vector<const EventDescriptor*> result;
        for (const auto& desc : events()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

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

    static std::vector<EventDescriptor>& events() {
        static std::vector<EventDescriptor> s_events = {

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

}