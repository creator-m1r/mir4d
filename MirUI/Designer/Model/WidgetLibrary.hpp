
#pragma once

#include "WidgetDescriptor.hpp"
#include "../../Core/Widget/WidgetFactory.hpp"
#include "../../Widgets/CheckBox/CheckBox.hpp"
#include "../../Widgets/TextField/TextField.hpp"
#include "../../Widgets/ComboBox/ComboBox.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

namespace MirUI {

class WidgetLibrary {
public:

    void registerWidget(const WidgetDescriptor& descriptor) {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [&](const WidgetDescriptor& d) { return d.type == descriptor.type; });
        if (it != m_widgets.end()) {
            *it = descriptor;
        } else {
            m_widgets.push_back(descriptor);
        }
    }

    [[nodiscard]] std::unique_ptr<Widget> create(WidgetType type) const {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [type](const WidgetDescriptor& d) { return d.type == type; });
        if (it != m_widgets.end() && it->factory) {
            return it->factory();
        }
        return WidgetFactory::create(type);
    }

    [[nodiscard]] const WidgetDescriptor* findByType(WidgetType type) const {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [type](const WidgetDescriptor& d) { return d.type == type; });
        return (it != m_widgets.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const WidgetDescriptor* findByName(const std::string& name) const {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [&name](const WidgetDescriptor& d) { return d.name == name; });
        return (it != m_widgets.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const std::vector<WidgetDescriptor>& widgets() const {
        return m_widgets;
    }

    void unregisterWidget(WidgetType type) {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [type](const WidgetDescriptor& d) { return d.type == type; });
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
        }
    }

    void clear() {
        m_widgets.clear();
    }

    void populateDefaults() {

        registerWidget(WidgetDescriptor(
            WidgetType::Button,
            "Кнопка",
            "button",
            []() -> std::unique_ptr<Widget> {
                auto btn = std::make_unique<Button>();
                btn->setText("Кнопка");
                btn->setLayoutData(LayoutData::fixed(120, 36));
                return btn;
            },
            {
                PropertyDescriptor("text", "Текст", PropertyType::String, StateValue(std::string("Кнопка"))),
                PropertyDescriptor("width", "Ширина", PropertyType::Float, StateValue(120.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Float, StateValue(36.0)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("cornerRadius", "Радиус скругления", PropertyType::Float, StateValue(8.0)),
                PropertyDescriptor("alignment", "Выравнивание", PropertyType::Enum, StateValue(std::string("Center")),
                                   std::vector<std::string>{"Left", "Center", "Right"})
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::Label,
            "Надпись",
            "label",
            []() -> std::unique_ptr<Widget> {
                auto lbl = std::make_unique<Label>();
                lbl->setText("Надпись");
                lbl->setLayoutData(LayoutData::fit());
                return lbl;
            },
            {
                PropertyDescriptor("text", "Текст", PropertyType::String, StateValue(std::string("Надпись"))),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("alignment", "Выравнивание", PropertyType::Enum, StateValue(std::string("Left")),
                                   std::vector<std::string>{"Left", "Center", "Right"})
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::Panel,
            "Контейнер",
            "panel",
            []() -> std::unique_ptr<Widget> {
                auto panel = std::make_unique<Widget>(WidgetType::Panel);
                panel->setLayoutData(LayoutData::fit());
                return panel;
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Float, StateValue(200.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Float, StateValue(200.0))
            },
            true,
            {}
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::Toolbar,
            "Панель инструментов",
            "toolbar",
            []() -> std::unique_ptr<Widget> {
                auto tb = std::make_unique<Toolbar>();
                tb->setLayoutData(LayoutData::fixed(0, 44));
                return tb;
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("height", "Высота", PropertyType::Float, StateValue(44.0))
            },
            true,
            {WidgetType::Button, WidgetType::Label, WidgetType::CheckBox, WidgetType::TextField, WidgetType::ComboBox}
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::Tree,
            "Дерево",
            "tree",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<Tree>();
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Float, StateValue(250.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Float, StateValue(400.0))
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::PropertyGrid,
            "Инспектор свойств",
            "propertygrid",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<PropertyGrid>();
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Float, StateValue(280.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Float, StateValue(500.0))
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::Viewport,
            "Вьюпорт",
            "viewport",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<Viewport>();
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("gridVisible", "Сетка", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("axesVisible", "Оси", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("gizmoVisible", "Гизмо", PropertyType::Boolean, StateValue(true))
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::DockPanel,
            "Стыкуемая панель",
            "dockpanel",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<Widget>(WidgetType::DockPanel);
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Float, StateValue(250.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Float, StateValue(400.0))
            },
            true,
            {}
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::CheckBox,
            "Флажок",
            "checkbox",
            []() -> std::unique_ptr<Widget> {
                auto cb = std::make_unique<CheckBox>();
                cb->setText("Флажок");
                cb->setLayoutData(LayoutData::fit());
                return cb;
            },
            {
                PropertyDescriptor("text", "Текст", PropertyType::String, StateValue(std::string("Флажок"))),
                PropertyDescriptor("checked", "Галочка", PropertyType::Boolean, StateValue(false)),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true))
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::TextField,
            "Текстовое поле",
            "textfield",
            []() -> std::unique_ptr<Widget> {
                auto tf = std::make_unique<TextField>();
                tf->setLayoutData(LayoutData::fixed(200, 28));
                return tf;
            },
            {
                PropertyDescriptor("text", "Текст", PropertyType::String, StateValue(std::string(""))),
                PropertyDescriptor("placeholder", "Подсказка", PropertyType::String, StateValue(std::string("Введите текст..."))),
                PropertyDescriptor("readOnly", "Только чтение", PropertyType::Boolean, StateValue(false)),
                PropertyDescriptor("maxLength", "Макс. длина", PropertyType::Integer, StateValue(static_cast<int64_t>(0))),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("textAlignment", "Выравнивание текста", PropertyType::Enum, StateValue(std::string("Left")),
                                   std::vector<std::string>{"Left", "Center", "Right"})
            }
        ));

        registerWidget(WidgetDescriptor(
            WidgetType::ComboBox,
            "Выпадающий список",
            "combobox",
            []() -> std::unique_ptr<Widget> {
                auto cb = std::make_unique<ComboBox>(std::vector<std::string>{"Вариант 1", "Вариант 2", "Вариант 3"});
                cb->setLayoutData(LayoutData::fixed(200, 28));
                return cb;
            },
            {
                PropertyDescriptor("items", "Элементы (через |)", PropertyType::String,
                                   StateValue(std::string("Вариант 1|Вариант 2|Вариант 3"))),
                PropertyDescriptor("selectedIndex", "Выбранный индекс", PropertyType::Integer, StateValue(static_cast<int64_t>(0))),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, StateValue(true)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, StateValue(true))
            }
        ));
    }

private:
    std::vector<WidgetDescriptor> m_widgets;
};

}