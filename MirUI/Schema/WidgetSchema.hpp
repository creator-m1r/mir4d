
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

struct WidgetDescriptor {
    WidgetType type;
    std::string id;
    std::string name;
    std::string description;
    std::string icon;
    bool isContainer = false;
    std::vector<WidgetType> allowedChildren;

    std::unordered_map<std::string, StateValue> defaultProperties;
};

class WidgetSchema {
public:

    [[nodiscard]] static const std::vector<WidgetDescriptor>& allWidgets() {
        return widgets();
    }

    [[nodiscard]] static const WidgetDescriptor* findById(const std::string& id) {
        auto& wgts = widgets();
        auto it = std::find_if(wgts.begin(), wgts.end(),
            [&id](const WidgetDescriptor& desc) { return desc.id == id; });
        return (it != wgts.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static const WidgetDescriptor* findByType(WidgetType type) {
        auto& wgts = widgets();
        auto it = std::find_if(wgts.begin(), wgts.end(),
            [type](const WidgetDescriptor& desc) { return desc.type == type; });
        return (it != wgts.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static bool canContain(WidgetType parentType, WidgetType childType) {
        auto parentDesc = findByType(parentType);
        if (!parentDesc || !parentDesc->isContainer) return false;
        const auto& allowed = parentDesc->allowedChildren;

        if (allowed.empty()) return true;
        return std::find(allowed.begin(), allowed.end(), childType) != allowed.end();
    }

    [[nodiscard]] static std::unordered_map<std::string, StateValue> defaultProperties(WidgetType type) {
        auto desc = findByType(type);
        if (desc) return desc->defaultProperties;
        return {};
    }

private:

    static std::vector<WidgetDescriptor>& widgets() {
        static std::vector<WidgetDescriptor> s_widgets = {

            {
                WidgetType::Window,
                "window",
                "Окно",
                "Корневое окно приложения. Содержит все остальные виджеты.",
                "window",
                true,
                {
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

            {
                WidgetType::Panel,
                "panel",
                "Контейнер",
                "Универсальный контейнер для группировки виджетов.",
                "panel",
                true,
                {},
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(200.0)},
                    {"height", StateValue(200.0)}
                }
            },

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

            {
                WidgetType::Button,
                "button",
                "Кнопка",
                "Нажимаемый элемент с текстом, иконкой и командой.",
                "button",
                false,
                {},
                {
                    {"text", StateValue(std::string("Кнопка"))},
                    {"width", StateValue(100.0)},
                    {"height", StateValue(32.0)},
                    {"visible", StateValue(true)},
                    {"enabled", StateValue(true)}
                }
            },

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

            {
                WidgetType::Tree,
                "tree",
                "Дерево",
                "Иерархический список элементов (навигатор, структура проекта).",
                "tree",
                false,
                {},
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(250.0)},
                    {"height", StateValue(500.0)},
                    {"multiSelect", StateValue(false)}
                }
            },

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
                    WidgetType::Toolbar
                },
                {
                    {"visible", StateValue(true)},
                    {"width", StateValue(0.0)},
                    {"height", StateValue(44.0)},
                    {"orientation", StateValue(std::string("horizontal"))}
                }
            },

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

}