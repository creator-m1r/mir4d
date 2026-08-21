// MirUI/Widgets/Container/Container.hpp
// 📦 Виджет-контейнер — основа для всех виджетов, которые могут содержать детей.
//
// Обычные виджеты (кнопка, надпись) не могут иметь внутри себя другие виджеты.
// Но панели, тулбары, окна и другие составные элементы — могут.
// Container — это специальный вид Widget, у которого эта возможность включена.
//
// Он наследует всё от базового Widget (id, bounds, видимость, свойства...),
// но дополнительно:
//   • всегда возвращает true на вопрос «можно ли добавить детей?»
//   • может проверять, разрешён ли конкретный тип виджета в качестве ребёнка
//     (используя WidgetSchema, чтобы, например, запретить добавлять кнопку внутрь кнопки)
//   • предоставляет удобные методы для массового добавления и удаления детей
//
// Все сложные виджеты-контейнеры в MirUI (Panel, Toolbar, DockPanel, Window)
// создаются как экземпляры Container с соответствующим WidgetType.
// Благодаря этому коду редактора не нужно знать особенности каждого типа —
// он работает с ними одинаково через базовый класс Container.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "../../Core/Widget/WidgetType.hpp"
#include "../../Schema/WidgetSchema.hpp"  // чтобы проверять allowedChildren
#include <vector>
#include <algorithm>

namespace MirUI {

class Container : public Widget {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает тип контейнера (Panel, Toolbar, DockPanel, Window...).
    // По умолчанию тип — Panel.
    explicit Container(WidgetType type = WidgetType::Panel)
        : Widget(type)
    {
        // Контейнеры по умолчанию не имеют фиксированного размера —
        // они обычно подстраиваются под детей или заполняют родителя.
        setLayoutData(LayoutData::fit());
    }

    // ── Работа с детьми ──────────────────────────────────────
    // Эти методы уже есть в Widget, но мы добавляем дополнительную проверку
    // на допустимость типа ребёнка перед добавлением.

    // Добавить виджет-ребёнка.
    // Возвращает true, если добавление прошло успешно.
    // Откажется добавлять, если тип ребёнка не разрешён в этом контейнере
    // (согласно WidgetSchema).
    bool addChild(Widget* child) {
        if (!child) return false;

        // Проверяем, разрешён ли такой тип виджета внутри этого контейнера.
        if (!canAcceptChildType(child->type())) {
            return false; // этот виджет нельзя сюда положить
        }

        // Вызываем базовый метод добавления (он перенесёт ребёнка от старого родителя).
        Widget::addChild(child);
        return true;
    }

    // Удалить ребёнка по ID. Возвращает true, если ребёнок был найден и отсоединён.
    bool removeChild(WidgetID id) {
        return Widget::removeChild(id);
    }

    // ── Проверка допустимости типов детей ────────────────────

    // Можно ли вообще добавлять детей в этот виджет? Контейнер всегда говорит «да».
    [[nodiscard]] bool acceptsChildren() const {
        return true;
    }

    // Можно ли добавить виджет конкретного типа?
    // Проверяет через WidgetSchema::canContain.
    [[nodiscard]] bool canAcceptChildType(WidgetType childType) const {
        return WidgetSchema::canContain(this->type(), childType);
    }

    // ── Удобные методы для работы с несколькими детьми ───────

    // Удалить всех детей (не удаляя их объекты, просто отсоединить).
    // Внимание: после вызова нужно вручную удалить объекты, если они были созданы в куче.
    void removeAllChildren() {
        while (!children().empty()) {
            removeChild(children().back()->id());
        }
    }

    // Получить количество детей.
    [[nodiscard]] size_t childCount() const {
        return children().size();
    }

    // ── Свойства, характерные для контейнеров ────────────────

    // Направление компоновки детей (по умолчанию вертикальное).
    // Может быть переопределено в потомках или через свойство "layoutDirection".
    [[nodiscard]] LayoutDirection layoutDirection() const {
        auto val = getProperty("layoutDirection");
        if (val.has_value() && std::holds_alternative<std::string>(*val)) {
            const std::string& dir = std::get<std::string>(*val);
            if (dir == "horizontal") return LayoutDirection::Horizontal;
        }
        return LayoutDirection::Vertical;
    }

    void setLayoutDirection(LayoutDirection dir) {
        setProperty("layoutDirection",
            StateValue(std::string(dir == LayoutDirection::Horizontal ? "horizontal" : "vertical")));
    }

    // Отступы (padding) контейнера — внутреннее пространство между границей и детьми.
    // Пока хранятся как четыре отдельных свойства, позже можно объединить в Insets.
    [[nodiscard]] Insets padding() const {
        return Insets{
            getDoubleProperty("paddingTop", 0.0),
            getDoubleProperty("paddingRight", 0.0),
            getDoubleProperty("paddingBottom", 0.0),
            getDoubleProperty("paddingLeft", 0.0)
        };
    }

    void setPadding(const Insets& insets) {
        setProperty("paddingTop",    StateValue(insets.top));
        setProperty("paddingRight",  StateValue(insets.right));
        setProperty("paddingBottom", StateValue(insets.bottom));
        setProperty("paddingLeft",   StateValue(insets.left));
    }

    // Межэлементный отступ (spacing) между детьми.
    [[nodiscard]] double spacing() const {
        return getDoubleProperty("spacing", 4.0);
    }

    void setSpacing(double spacing) {
        setProperty("spacing", StateValue(spacing));
    }

private:
    // Вспомогательный метод для получения double-свойств с значением по умолчанию.
    double getDoubleProperty(const std::string& name, double defaultValue) const {
        auto val = getProperty(name);
        if (val.has_value()) {
            if (std::holds_alternative<double>(*val))
                return std::get<double>(*val);
            if (std::holds_alternative<int64_t>(*val))
                return static_cast<double>(std::get<int64_t>(*val));
        }
        return defaultValue;
    }
};

} // namespace MirUI