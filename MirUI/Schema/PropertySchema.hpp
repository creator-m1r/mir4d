
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include "../Core/State/StateValue.hpp"

namespace MirUI {

enum class PropertyValueType {
    String,
    Integer,
    Float,
    Boolean,
    Color,
    Font,
    Enum,
    Size,
    Rect
};

struct PropertyDescriptor {
    std::string id;
    PropertyValueType type;
    std::string category;
    std::string name;
    std::string description;
    StateValue defaultValue;
    std::vector<std::string> enumValues;
};

class PropertySchema {
public:

    [[nodiscard]] static const std::vector<PropertyDescriptor>& allProperties() {
        return properties();
    }

    [[nodiscard]] static const PropertyDescriptor* find(const std::string& id) {
        auto& props = properties();
        auto it = std::find_if(props.begin(), props.end(),
            [&id](const PropertyDescriptor& desc) { return desc.id == id; });
        return (it != props.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static std::vector<const PropertyDescriptor*> findByCategory(const std::string& category) {
        std::vector<const PropertyDescriptor*> result;
        for (const auto& desc : properties()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

    [[nodiscard]] static std::vector<std::string> allCategories() {
        std::vector<std::string> categories;
        for (const auto& desc : properties()) {
            if (std::find(categories.begin(), categories.end(), desc.category) == categories.end()) {
                categories.push_back(desc.category);
            }
        }
        return categories;
    }

    [[nodiscard]] static std::optional<StateValue> defaultValue(const std::string& id) {
        auto desc = find(id);
        if (desc) {
            return desc->defaultValue;
        }
        return std::nullopt;
    }

private:

    static std::vector<PropertyDescriptor>& properties() {
        static std::vector<PropertyDescriptor> s_properties = {

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
                StateValue(std::string("System;14;400;0")),
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

}