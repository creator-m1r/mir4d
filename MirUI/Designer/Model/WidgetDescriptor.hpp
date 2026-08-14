// MirUI/Designer/Model/WidgetCatalogDescriptor.hpp
// 🏷️ Дескриптор виджета — "паспорт" типа виджета для каталога (WidgetLibrary).
//
// Чтобы панель инструментов (Toolbox) могла показать список доступных виджетов
// и создавать их, она должна знать о каждом типе WidgetType:
//   • Как он называется (по-русски).
//   • Какую иконку показывать.
//   • Как создать экземпляр (фабрика).
//   • Какие свойства у него есть (чтобы инспектор мог их редактировать).
//
// WidgetCatalogDescriptor хранит всю эту информацию в одном месте.
// Благодаря ему добавление нового типа виджета в MirUI Designer
// сводится к созданию одного объекта WidgetCatalogDescriptor и регистрации
// его в WidgetLibrary. Вся остальная логика (отображение в тулбоксе,
// создание через фабрику, настройка инспектора) работает автоматически.
//
// Поля:
//   • type       — тип виджета из перечисления WidgetType
//   • name       — человекочитаемое название (например, "Кнопка")
//   • icon       — идентификатор иконки (платформонезависимая строка)
//   • factory    — функция, создающая новый экземпляр виджета
//   • properties — список дескрипторов свойств (PropertyDescriptor),
//                  которые InspectorModel будет показывать для этого типа
//   • isContainer — может ли виджет содержать детей (контейнер)
//   • allowedChildren — типы виджетов, которые можно поместить внутрь (если isContainer)
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "../../Core/Widget/WidgetType.hpp"
#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/PropertyDescriptor.hpp" // наш универсальный дескриптор свойства
#include "../../Foundation/Icons/IconID.hpp"

namespace MirUI {

struct WidgetCatalogDescriptor {
    WidgetType type;                              // тип виджета (Button, Label, Tree…)
    std::string name;                             // человекочитаемое имя ("Кнопка", "Надпись")
    std::string icon;                             // идентификатор иконки (например, "button")
    std::function<std::unique_ptr<Widget>()> factory; // функция-фабрика для создания экземпляра

    // Список свойств, которые Inspector будет показывать для этого типа виджета.
    // Каждый PropertyDescriptor описывает одно свойство: имя, тип, значение по умолчанию.
    std::vector<PropertyDescriptor> properties;

    bool isContainer = false;                     // может ли этот виджет содержать детей
    std::vector<WidgetType> allowedChildren;      // разрешённые типы детей (если isContainer)

    // ── Конструкторы для удобства ────────────────────────────
    WidgetCatalogDescriptor() = default;

    // Упрощённый конструктор без свойств (свойства можно добавить потом).
    WidgetCatalogDescriptor(WidgetType type,
                     std::string name,
                     std::string icon,
                     std::function<std::unique_ptr<Widget>()> factory)
        : type(type)
        , name(std::move(name))
        , icon(std::move(icon))
        , factory(std::move(factory))
    {}

    // Полный конструктор.
    WidgetCatalogDescriptor(WidgetType type,
                     std::string name,
                     std::string icon,
                     std::function<std::unique_ptr<Widget>()> factory,
                     std::vector<PropertyDescriptor> properties,
                     bool isContainer = false,
                     std::vector<WidgetType> allowedChildren = {})
        : type(type)
        , name(std::move(name))
        , icon(std::move(icon))
        , factory(std::move(factory))
        , properties(std::move(properties))
        , isContainer(isContainer)
        , allowedChildren(std::move(allowedChildren))
    {}

    // Создаёт экземпляр виджета, используя фабрику.
    [[nodiscard]] std::unique_ptr<Widget> create() const {
        if (factory) {
            return factory();
        }
        return nullptr;
    }
};

} // namespace MirUI