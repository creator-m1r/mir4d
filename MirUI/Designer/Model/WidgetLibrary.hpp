// MirUI/Designer/Model/WidgetLibrary.hpp — обновлённый каталог с новыми виджетами.
// Добавлены CheckBox, TextField, ComboBox.
//
// Чистый C++23, без платформенных зависимостей.

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
    // ── Регистрация виджета ──────────────────────────────────
    void registerWidget(const WidgetCatalogDescriptor& descriptor) {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [&](const WidgetCatalogDescriptor& d) { return d.type == descriptor.type; });
        if (it != m_widgets.end()) {
            *it = descriptor;
        } else {
            m_widgets.push_back(descriptor);
        }
    }

    // ── Создание экземпляра ──────────────────────────────────
    [[nodiscard]] std::unique_ptr<Widget> create(WidgetType type) const {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [type](const WidgetCatalogDescriptor& d) { return d.type == type; });
        if (it != m_widgets.end() && it->factory) {
            return it->factory();
        }
        return WidgetFactory::create(type);
    }

    // ── Поиск дескриптора ────────────────────────────────────
    [[nodiscard]] const WidgetCatalogDescriptor* findByType(WidgetType type) const {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [type](const WidgetCatalogDescriptor& d) { return d.type == type; });
        return (it != m_widgets.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const WidgetCatalogDescriptor* findByName(const std::string& name) const {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [&name](const WidgetCatalogDescriptor& d) { return d.name == name; });
        return (it != m_widgets.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const std::vector<WidgetCatalogDescriptor>& widgets() const {
        return m_widgets;
    }

    void unregisterWidget(WidgetType type) {
        auto it = std::find_if(m_widgets.begin(), m_widgets.end(),
            [type](const WidgetCatalogDescriptor& d) { return d.type == type; });
        if (it != m_widgets.end()) {
            m_widgets.erase(it);
        }
    }

    void clear() {
        m_widgets.clear();
    }

    // ── Заполнение стандартным набором (с новыми виджетами) ──
    void populateDefaults() {
        // Кнопка
        registerWidget(WidgetCatalogDescriptor(
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
                PropertyDescriptor("text", "Текст", PropertyType::String, PropertyValue(std::string("Кнопка"))),
                PropertyDescriptor("width", "Ширина", PropertyType::Number, PropertyValue(120.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Number, PropertyValue(36.0)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("cornerRadius", "Радиус скругления", PropertyType::Number, PropertyValue(8.0)),
                PropertyDescriptor("alignment", "Выравнивание", PropertyType::Enum, PropertyValue(std::string("Center")),
                                   std::vector<std::string>{"Left", "Center", "Right"})
            }
        ));

        // Надпись
        registerWidget(WidgetCatalogDescriptor(
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
                PropertyDescriptor("text", "Текст", PropertyType::String, PropertyValue(std::string("Надпись"))),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("alignment", "Выравнивание", PropertyType::Enum, PropertyValue(std::string("Left")),
                                   std::vector<std::string>{"Left", "Center", "Right"})
            }
        ));

        // Контейнер (Panel)
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::Panel,
            "Контейнер",
            "panel",
            []() -> std::unique_ptr<Widget> {
                auto panel = std::make_unique<Widget>(WidgetType::Panel);
                panel->setLayoutData(LayoutData::fit());
                return panel;
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Number, PropertyValue(200.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Number, PropertyValue(200.0))
            },
            true, // контейнер
            {}
        ));

        // Панель инструментов (Toolbar)
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::Toolbar,
            "Панель инструментов",
            "toolbar",
            []() -> std::unique_ptr<Widget> {
                auto tb = std::make_unique<Toolbar>();
                tb->setLayoutData(LayoutData::fixed(0, 44));
                return tb;
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("height", "Высота", PropertyType::Number, PropertyValue(44.0))
            },
            true,
            {WidgetType::Button, WidgetType::Label, WidgetType::CheckBox, WidgetType::TextField, WidgetType::ComboBox}
        ));

        // Дерево
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::Tree,
            "Дерево",
            "tree",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<Tree>();
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Number, PropertyValue(250.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Number, PropertyValue(400.0))
            }
        ));

        // Инспектор свойств
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::PropertyGrid,
            "Инспектор свойств",
            "propertygrid",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<PropertyGrid>();
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Number, PropertyValue(280.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Number, PropertyValue(500.0))
            }
        ));

        // Вьюпорт
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::Viewport,
            "Вьюпорт",
            "viewport",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<Viewport>();
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("gridVisible", "Сетка", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("axesVisible", "Оси", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("gizmoVisible", "Гизмо", PropertyType::Boolean, PropertyValue(true))
            }
        ));

        // Стыкуемая панель
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::DockPanel,
            "Стыкуемая панель",
            "dockpanel",
            []() -> std::unique_ptr<Widget> {
                return std::make_unique<Widget>(WidgetType::DockPanel);
            },
            {
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("width", "Ширина", PropertyType::Number, PropertyValue(250.0)),
                PropertyDescriptor("height", "Высота", PropertyType::Number, PropertyValue(400.0))
            },
            true,
            {}
        ));

        // ── НОВЫЕ ВИДЖЕТЫ ─────────────────────────────────────

        // Флажок (CheckBox)
        registerWidget(WidgetCatalogDescriptor(
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
                PropertyDescriptor("text", "Текст", PropertyType::String, PropertyValue(std::string("Флажок"))),
                PropertyDescriptor("checked", "Галочка", PropertyType::Boolean, PropertyValue(false)),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true))
            }
        ));

        // Текстовое поле (TextField)
        registerWidget(WidgetCatalogDescriptor(
            WidgetType::TextField,
            "Текстовое поле",
            "textfield",
            []() -> std::unique_ptr<Widget> {
                auto tf = std::make_unique<TextField>();
                tf->setLayoutData(LayoutData::fixed(200, 28));
                return tf;
            },
            {
                PropertyDescriptor("text", "Текст", PropertyType::String, PropertyValue(std::string(""))),
                PropertyDescriptor("placeholder", "Подсказка", PropertyType::String, PropertyValue(std::string("Введите текст..."))),
                PropertyDescriptor("readOnly", "Только чтение", PropertyType::Boolean, PropertyValue(false)),
                PropertyDescriptor("maxLength", "Макс. длина", PropertyType::Integer, PropertyValue(static_cast<int64_t>(0))),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("textAlignment", "Выравнивание текста", PropertyType::Enum, PropertyValue(std::string("Left")),
                                   std::vector<std::string>{"Left", "Center", "Right"})
            }
        ));

        // Выпадающий список (ComboBox)
        registerWidget(WidgetCatalogDescriptor(
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
                                   PropertyValue(std::string("Вариант 1|Вариант 2|Вариант 3"))),
                PropertyDescriptor("selectedIndex", "Выбранный индекс", PropertyType::Integer, PropertyValue(static_cast<int64_t>(0))),
                PropertyDescriptor("enabled", "Доступность", PropertyType::Boolean, PropertyValue(true)),
                PropertyDescriptor("visible", "Видимость", PropertyType::Boolean, PropertyValue(true))
            }
        ));
    }

private:
    std::vector<WidgetCatalogDescriptor> m_widgets;
};

} // namespace MirUI