// MirUI/Schema/WidgetSchema.hpp
// 🧩 Схема виджетов MirUI — центральный реестр всех стандартных типов виджетов.
//
// Когда ты открываешь панель инструментов (Toolbox) или инспектор (Inspector),
// система должна знать, какие вообще бывают виджеты: как они называются,
// какие у них свойства по умолчанию, какие дети им разрешены, можно ли
// в них вкладывать другие виджеты. WidgetSchema хранит все эти знания
// в одном месте — это как энциклопедия видовжетов.
//
// Каждая запись (WidgetDescriptor) описывает один тип виджета:
//   • type          — значение из перечисления WidgetType (Button, Label, …)
//   • id            — строковый идентификатор (например, "button", "tree")
//   • name          — отображаемое имя на русском («Кнопка», «Дерево»)
//   • description   — подробное описание (тултип)
//   • icon          — идентификатор иконки для тулбокса (платформонезависимый)
//   • defaultProperties — список пар (имя свойства, значение по умолчанию),
//                          которые устанавливаются при создании виджета
//   • allowedChildren   — список WidgetType, которые разрешено добавлять
//                          внутрь этого виджета (если он контейнер)
//   • isContainer   — может ли виджет содержать дочерние элементы
//
// Благодаря этой схеме:
//   • Toolbox автоматически наполняется списком доступных виджетов.
//   • AddWidgetCommand знает, можно ли добавить кнопку внутрь тулбара.
//   • InspectorModel показывает осмысленные названия и категории.
//   • Renderer может проверить, допустима ли такая вложенность.
//   • При создании нового виджета все его свойства заполняются дефолтными
//     значениями из схемы.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Core/Widget/WidgetType.hpp"
#include "../Core/State/StateValue.hpp"
#include "../Foundation/Icons/IconID.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <unordered_map>

namespace MirUI {

// ── Дескриптор одного типа виджета ─────────────────────────
struct WidgetDescriptor {
    WidgetType type;                              // тип из перечисления WidgetType
    std::string id;                               // строковый идентификатор (напр. "button")
    std::string name;                             // имя на русском ("Кнопка")
    std::string description;                      // описание для тултипа
    std::string icon;                             // идентификатор иконки
    bool isContainer = false;                     // может ли содержать детей
    std::vector<WidgetType> allowedChildren;      // разрешённые типы детей (если контейнер)

    // Свойства, которые автоматически устанавливаются при создании виджета.
    // Ключ — имя свойства (напр. "text", "width"), значение — значение по умолчанию.
    std::unordered_map<std::string, StateValue> defaultProperties;
};

// ── Схема (реестр) виджетов ────────────────────────────────
class WidgetSchema {
public:
    // Получить список всех зарегистрированных типов виджетов.
    [[nodiscard]] static const std::vector<WidgetDescriptor>& allWidgets() {
        return widgets();
    }

    // Найти дескриптор виджета по его строковому идентификатору.
    [[nodiscard]] static const WidgetDescriptor* findById(const std::string& id) {
        auto& wgts = widgets();
        auto it = std::find_if(wgts.begin(), wgts.end(),
            [&id](const WidgetDescriptor& desc) { return desc.id == id; });
        return (it != wgts.end()) ? &(*it) : nullptr;
    }

    // Найти дескриптор виджета по WidgetType.
    [[nodiscard]] static const WidgetDescriptor* findByType(WidgetType type) {
        auto& wgts = widgets();
        auto it = std::find_if(wgts.begin(), wgts.end(),
            [type](const WidgetDescriptor& desc) { return desc.type == type; });
        return (it != wgts.end()) ? &(*it) : nullptr;
    }

    // Проверить, можно ли добавить виджет типа childType внутрь контейнера типа parentType.
    [[nodiscard]] static bool canContain(WidgetType parentType, WidgetType childType) {
        auto parentDesc = findByType(parentType);
        if (!parentDesc || !parentDesc->isContainer) return false;
        const auto& allowed = parentDesc->allowedChildren;
        // Если список разрешённых пуст, значит можно добавлять любые виджеты.
        if (allowed.empty()) return true;
        return std::find(allowed.begin(), allowed.end(), childType) != allowed.end();
    }

    // Получить свойства по умолчанию для указанного типа виджета.
    [[nodiscard]] static std::unordered_map<std::string, StateValue> defaultProperties(WidgetType type) {
        auto desc = findByType(type);
        if (desc) return desc->defaultProperties;
        return {};
    }

private:
    // Статический реестр всех встроенных типов виджетов.
    static std::vector<WidgetDescriptor>& widgets() {
        static std::vector<WidgetDescriptor> s_widgets = {
            // ── Окно (Window) ─────────────────────────────────
            {
                WidgetType::Window,
                "window",
                "Окно",
                "Корневое окно приложения. Содержит все остальные виджеты.",
                "window",
                true, // контейнер
                {    // разрешённые дети: панели, тулбар, вьюпорт, другие окна (MDI)
                    WidgetType::Panel,
                    WidgetType::DockPanel,
                    WidgetType::Toolbar,
                    WidgetType::Ribbon,
                    WidgetType::Viewport,
                    WidgetType::Window
                },
                {
                    {"width", StateValue(800.0)},
                    {"height", StateValue(600.0)},
                    {"visible", StateValue(true)}
                }
            },

            // ── Контейнер (Panel) ────────────────────────────
            {
                WidgetType::Panel,
                "panel",
                "Контейнер",
                "Универсальный контейнер для группировки виджетов.",
                "panel",
                true,
                {}, // разрешены любые дети
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(200.0)},
                    {"height", StateValue(200.0)}
                }
            },

            // ── Стыкуемая панель (DockPanel) ─────────────────
            {
                WidgetType::DockPanel,
                "dockpanel",
                "Стыкуемая панель",
                "Панель, которую можно прикреплять к краям окна или делать плавающей.",
                "dockpanel",
                true,
                {
                    WidgetType::Tree,
                    WidgetType::PropertyGrid,
                    WidgetType::Panel,
                    WidgetType::Label,
                    WidgetType::Button
                },
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(250.0)},
                    {"height", StateValue(400.0)},
                    {"floatable", StateValue(true)},
                    {"closable", StateValue(true)}
                }
            },

            // ── Кнопка (Button) ──────────────────────────────
            {
                WidgetType::Button,
                "button",
                "Кнопка",
                "Нажимаемый элемент с текстом, иконкой и командой.",
                "button",
                false, // не контейнер
                {},
                {
                    {"text", StateValue(std::string("Кнопка"))},
                    {"width", StateValue(100.0)},
                    {"height", StateValue(32.0)},
                    {"visible", StateValue(true)},
                    {"enabled", StateValue(true)}
                }
            },

            // ── Надпись (Label) ──────────────────────────────
            {
                WidgetType::Label,
                "label",
                "Надпись",
                "Простой текст для отображения подписей и сообщений.",
                "label",
                false,
                {},
                {
                    {"text", StateValue(std::string("Надпись"))},
                    {"visible", StateValue(true)},
                    {"alignment", StateValue(std::string("Left"))}
                }
            },

            // ── Дерево (Tree) ────────────────────────────────
            {
                WidgetType::Tree,
                "tree",
                "Дерево",
                "Иерархический список элементов (навигатор, структура проекта).",
                "tree",
                false, // у дерева свои дочерние элементы (TreeNode), не виджеты
                {},
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(250.0)},
                    {"height", StateValue(500.0)},
                    {"multiSelect", StateValue(false)}
                }
            },

            // ── Инспектор свойств (PropertyGrid) ─────────────
            {
                WidgetType::PropertyGrid,
                "propertygrid",
                "Инспектор свойств",
                "Панель для редактирования свойств выделенного объекта.",
                "propertygrid",
                false,
                {},
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(300.0)},
                    {"height", StateValue(500.0)}
                }
            },

            // ── Панель инструментов (Toolbar) ────────────────
            {
                WidgetType::Toolbar,
                "toolbar",
                "Панель инструментов",
                "Контейнер для кнопок и других быстрых действий.",
                "toolbar",
                true,
                {
                    WidgetType::Button,
                    WidgetType::Label,
                    WidgetType::Toolbar // вложенные тулбары (редко, но возможно)
                },
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(0.0)},   // обычно растягивается
                    {"height", StateValue(44.0)},
                    {"orientation", StateValue(std::string("horizontal"))}
                }
            },

            // ── Лента (Ribbon) ───────────────────────────────
            {
                WidgetType::Ribbon,
                "ribbon",
                "Лента",
                "Лента инструментов в стиле Microsoft Office.",
                "ribbon",
                true,
                { WidgetType::Toolbar, WidgetType::Button, WidgetType::Label },
                {
                    {"visible", StateValue(true)},
                    {"height", StateValue(120.0)}
                }
            },

            // ── Вьюпорт (Viewport) ───────────────────────────
            {
                WidgetType::Viewport,
                "viewport",
                "Вьюпорт",
                "Область для отображения 3D-сцены.",
                "viewport",
                false,
                {},
                {
                    {"visible", StateValue(true)},
                    {"gridVisible", StateValue(true)},
                    {"axesVisible", StateValue(true)},
                    {"gizmoVisible", StateValue(true)}
                }
            },

            // ── Таймлайн (Timeline) ───────────────────────────
            {
                WidgetType::Timeline,
                "timeline",
                "Таймлайн",
                "Временная шкала для анимации и работы с 4D-данными.",
                "timeline",
                false,
                {},
                {
                    {"visible", StateValue(false)},
                    {"height", StateValue(200.0)}
                }
            }
        };
        return s_widgets;
    }
};

} // namespace MirUI