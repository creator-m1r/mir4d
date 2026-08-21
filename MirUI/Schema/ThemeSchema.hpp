
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "../Core/State/StateValue.hpp"

namespace MirUI {

enum class ThemeValueType {
    Color,
    Double,
    Font
};

struct ThemeDescriptor {
    std::string id;
    ThemeValueType type;
    std::string category;
    std::string name;
    std::string description;
    StateValue defaultValue;
};

class ThemeSchema {
public:

    [[nodiscard]] static const std::vector<ThemeDescriptor>& allTokens() {
        return tokens();
    }

    [[nodiscard]] static const ThemeDescriptor* find(const std::string& id) {
        auto& toks = tokens();
        auto it = std::find_if(toks.begin(), toks.end(),
            [&id](const ThemeDescriptor& desc) { return desc.id == id; });
        return (it != toks.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static std::vector<const ThemeDescriptor*> findByCategory(const std::string& category) {
        std::vector<const ThemeDescriptor*> result;
        for (const auto& desc : tokens()) {
            if (desc.category == category) {
                result.push_back(&desc);
            }
        }
        return result;
    }

    [[nodiscard]] static std::vector<std::string> allCategories() {
        std::vector<std::string> categories;
        for (const auto& desc : tokens()) {
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

    static std::vector<ThemeDescriptor>& tokens() {
        static std::vector<ThemeDescriptor> s_tokens = {

            {
                "colors.background",
                ThemeValueType::Color,
                "Цвета",
                "Фон",
                "Основной цвет фона всего приложения",
                StateValue(std::string("#F5F5F5FF"))
            },
            {
                "colors.surface",
                ThemeValueType::Color,
                "Цвета",
                "Поверхность",
                "Цвет панелей, карточек и других возвышающихся элементов",
                StateValue(std::string("#FFFFFFFF"))
            },
            {
                "colors.surfaceHover",
                ThemeValueType::Color,
                "Цвета",
                "Поверхность (наведение)",
                "Цвет поверхности при наведении курсора",
                StateValue(std::string("#F0F0F0FF"))
            },
            {
                "colors.surfaceActive",
                ThemeValueType::Color,
                "Цвета",
                "Поверхность (нажатие)",
                "Цвет поверхности при нажатии",
                StateValue(std::string("#E5E5E5FF"))
            },
            {
                "colors.textPrimary",
                ThemeValueType::Color,
                "Цвета",
                "Основной текст",
                "Цвет основного текста (заголовки, важный контент)",
                StateValue(std::string("#1A1A1AFF"))
            },
            {
                "colors.textSecondary",
                ThemeValueType::Color,
                "Цвета",
                "Вторичный текст",
                "Цвет второстепенного текста (подписи, подсказки)",
                StateValue(std::string("#666666FF"))
            },
            {
                "colors.textMuted",
                ThemeValueType::Color,
                "Цвета",
                "Приглушённый текст",
                "Цвет малозаметного текста (плейсхолдеры, неактивные элементы)",
                StateValue(std::string("#999999FF"))
            },
            {
                "colors.accent",
                ThemeValueType::Color,
                "Цвета",
                "Акцент",
                "Основной акцентный цвет (кнопки, выделение)",
                StateValue(std::string("#007AFFFF"))
            },
            {
                "colors.accentHover",
                ThemeValueType::Color,
                "Цвета",
                "Акцент (наведение)",
                "Цвет акцентного элемента при наведении",
                StateValue(std::string("#0062CCFF"))
            },
            {
                "colors.accentActive",
                ThemeValueType::Color,
                "Цвета",
                "Акцент (нажатие)",
                "Цвет акцентного элемента при нажатии",
                StateValue(std::string("#004C99FF"))
            },
            {
                "colors.border",
                ThemeValueType::Color,
                "Цвета",
                "Граница",
                "Цвет разделительных линий и границ",
                StateValue(std::string("#D9D9D9FF"))
            },
            {
                "colors.error",
                ThemeValueType::Color,
                "Цвета",
                "Ошибка",
                "Цвет для обозначения ошибок",
                StateValue(std::string("#E53935FF"))
            },
            {
                "colors.warning",
                ThemeValueType::Color,
                "Цвета",
                "Предупреждение",
                "Цвет для предупреждений",
                StateValue(std::string("#FB8C00FF"))
            },
            {
                "colors.success",
                ThemeValueType::Color,
                "Цвета",
                "Успех",
                "Цвет для успешных состояний",
                StateValue(std::string("#43A047FF"))
            },

            {
                "metrics.spacingXS",
                ThemeValueType::Double,
                "Метрики",
                "Отступ XS",
                "Очень маленький отступ (4 пикселя)",
                StateValue(4.0)
            },
            {
                "metrics.spacingS",
                ThemeValueType::Double,
                "Метрики",
                "Отступ S",
                "Маленький отступ (8 пикселей)",
                StateValue(8.0)
            },
            {
                "metrics.spacingM",
                ThemeValueType::Double,
                "Метрики",
                "Отступ M",
                "Средний отступ (12 пикселей)",
                StateValue(12.0)
            },
            {
                "metrics.spacingL",
                ThemeValueType::Double,
                "Метрики",
                "Отступ L",
                "Большой отступ (16 пикселей)",
                StateValue(16.0)
            },
            {
                "metrics.spacingXL",
                ThemeValueType::Double,
                "Метрики",
                "Отступ XL",
                "Очень большой отступ (24 пикселя)",
                StateValue(24.0)
            },
            {
                "metrics.radiusS",
                ThemeValueType::Double,
                "Метрики",
                "Радиус S",
                "Маленькое скругление углов (4 пикселя)",
                StateValue(4.0)
            },
            {
                "metrics.radiusM",
                ThemeValueType::Double,
                "Метрики",
                "Радиус M",
                "Среднее скругление углов (8 пикселей)",
                StateValue(8.0)
            },
            {
                "metrics.radiusL",
                ThemeValueType::Double,
                "Метрики",
                "Радиус L",
                "Большое скругление углов (12 пикселей)",
                StateValue(12.0)
            },
            {
                "metrics.borderWidth",
                ThemeValueType::Double,
                "Метрики",
                "Толщина границы",
                "Стандартная толщина рамок (1 пиксель)",
                StateValue(1.0)
            },
            {
                "metrics.controlHeight",
                ThemeValueType::Double,
                "Метрики",
                "Высота контрола",
                "Стандартная высота элементов управления (кнопок, полей) — 28 пикселей",
                StateValue(28.0)
            },
            {
                "metrics.toolbarHeight",
                ThemeValueType::Double,
                "Метрики",
                "Высота тулбара",
                "Высота панели инструментов — 44 пикселя",
                StateValue(44.0)
            },
            {
                "metrics.panelWidth",
                ThemeValueType::Double,
                "Метрики",
                "Ширина панели",
                "Ширина боковых панелей (навигатор, инспектор) — 260 пикселей",
                StateValue(260.0)
            },

            {
                "typography.title",
                ThemeValueType::Font,
                "Типографика",
                "Заголовок",
                "Шрифт для заголовков (крупный, жирный)",
                StateValue(std::string("System;24.0;700;0"))
            },
            {
                "typography.subtitle",
                ThemeValueType::Font,
                "Типографика",
                "Подзаголовок",
                "Шрифт для подзаголовков (средний, полужирный)",
                StateValue(std::string("System;18.0;500;0"))
            },
            {
                "typography.body",
                ThemeValueType::Font,
                "Типографика",
                "Основной текст",
                "Шрифт для основного текста (обычный)",
                StateValue(std::string("System;14.0;400;0"))
            },
            {
                "typography.caption",
                ThemeValueType::Font,
                "Типографика",
                "Подпись",
                "Мелкий шрифт для подписей и сносок",
                StateValue(std::string("System;12.0;400;0"))
            },
            {
                "typography.button",
                ThemeValueType::Font,
                "Типографика",
                "Кнопка",
                "Шрифт для текста на кнопках",
                StateValue(std::string("System;14.0;500;0"))
            },
            {
                "typography.code",
                ThemeValueType::Font,
                "Типографика",
                "Код",
                "Моноширинный шрифт для отображения кода",
                StateValue(std::string("Menlo;13.0;400;0"))
            }
        };
        return s_tokens;
    }
};

}