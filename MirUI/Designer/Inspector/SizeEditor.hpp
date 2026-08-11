// MirUI/Designer/Inspector/SizeEditor.hpp
// 📐 Редактор размеров для инспектора свойств (PropertyGrid).
//
// Управляет группой геометрических свойств выделенного виджета:
//   • Ширина и высота (с единицами измерения: px, %, auto)
//   • Политики растяжения/сжатия (Fixed, Fill, Fit, Stretch)
//   • Минимальные и максимальные ограничения размеров
//
// SizeEditor объединяет эти связанные поля в один логический блок,
// чтобы инспектор мог отобразить их компактно — например, два поля
// «Ширина» и «Высота» с выпадающими списками единиц рядом.
// Каждое изменение значения, единицы или политики создаёт отдельную
// команду (ChangePropertyCommand) и попадает в историю Undo/Redo.
//
// SizeEditor не занимается отрисовкой — он только хранит состояние
// и предоставляет методы для его изменения. Платформенный рендерер
// может использовать эти данные для построения интерфейса инспектора.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Layout/LayoutData.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MirUI {

class SizeEditor {
public:
    // Конструктор: запоминает документ и виджет, загружает текущие значения из LayoutData.
    SizeEditor(UIDocument& doc, WidgetID widgetId)
        : m_doc(doc)
        , m_widgetId(widgetId)
    {
        Widget* widget = doc.widgetTree().find(widgetId);
        if (widget) {
            m_layoutData = widget->layoutData(); // копируем текущие настройки
        } else {
            m_layoutData = LayoutData::fit(); // если виджет не найден, ставим авто
        }
    }

    // ── Текущие значения (для отображения) ──────────────────

    [[nodiscard]] double widthValue() const { return m_layoutData.widthValue; }
    [[nodiscard]] Unit   widthUnit() const { return m_layoutData.widthUnit; }
    [[nodiscard]] double heightValue() const { return m_layoutData.heightValue; }
    [[nodiscard]] Unit   heightUnit() const { return m_layoutData.heightUnit; }

    [[nodiscard]] SizePolicy horizontalPolicy() const { return m_layoutData.horizontalPolicy; }
    [[nodiscard]] SizePolicy verticalPolicy() const { return m_layoutData.verticalPolicy; }

    [[nodiscard]] double minWidth() const { return m_layoutData.minimumSize.width; }
    [[nodiscard]] double minHeight() const { return m_layoutData.minimumSize.height; }
    [[nodiscard]] double maxWidth() const { return m_layoutData.maximumSize.width; }
    [[nodiscard]] double maxHeight() const { return m_layoutData.maximumSize.height; }

    // ── Установка новых значений (создают команды) ──────────

    void setWidth(double value)    { changeProperty("width", StateValue(value)); }
    void setHeight(double value)   { changeProperty("height", StateValue(value)); }
    void setMinWidth(double value) { changeProperty("minWidth", StateValue(value)); }
    void setMinHeight(double value){ changeProperty("minHeight", StateValue(value)); }
    void setMaxWidth(double value) { changeProperty("maxWidth", StateValue(value)); }
    void setMaxHeight(double value){ changeProperty("maxHeight", StateValue(value)); }

    // Для единиц измерения и политик нужна специальная обработка,
    // так как они не являются простыми числами. Мы будем использовать
    // ChangePropertyCommand с ключами "widthUnit", "heightUnit" и т.д.
    // Эти ключи обрабатываются в Widget::setProperty (мы добавим их позже),
    // а пока SizeEditor напрямую модифицирует LayoutData через отдельную команду.
    void setWidthUnit(Unit unit) {
        executeLayoutCommand([this, unit]() {
            m_layoutData.widthUnit = unit;
        }, [this, oldUnit = m_layoutData.widthUnit]() {
            m_layoutData.widthUnit = oldUnit;
        });
    }
    void setHeightUnit(Unit unit) {
        executeLayoutCommand([this, unit]() {
            m_layoutData.heightUnit = unit;
        }, [this, oldUnit = m_layoutData.heightUnit]() {
            m_layoutData.heightUnit = oldUnit;
        });
    }
    void setHorizontalPolicy(SizePolicy policy) {
        executeLayoutCommand([this, policy]() {
            m_layoutData.horizontalPolicy = policy;
        }, [this, oldPolicy = m_layoutData.horizontalPolicy]() {
            m_layoutData.horizontalPolicy = oldPolicy;
        });
    }
    void setVerticalPolicy(SizePolicy policy) {
        executeLayoutCommand([this, policy]() {
            m_layoutData.verticalPolicy = policy;
        }, [this, oldPolicy = m_layoutData.verticalPolicy]() {
            m_layoutData.verticalPolicy = oldPolicy;
        });
    }

    // ── Вспомогательные списки для EnumEditor ────────────────
    // Возвращает список строк-представлений единиц измерения.
    [[nodiscard]] static std::vector<std::string> unitNames() {
        return {"px", "%", "Auto"};
    }

    // Возвращает список строк-представлений политик размера.
    [[nodiscard]] static std::vector<std::string> policyNames() {
        return {"Fixed", "Fill", "Fit", "Stretch"};
    }

    // Преобразование строки в Unit (для EnumEditor).
    [[nodiscard]] static Unit unitFromName(const std::string& name) {
        if (name == "px")   return Unit::Pixel;
        if (name == "%")    return Unit::Percent;
        if (name == "Auto") return Unit::Auto;
        return Unit::Auto; // по умолчанию
    }

    // Преобразование строки в SizePolicy.
    [[nodiscard]] static SizePolicy policyFromName(const std::string& name) {
        if (name == "Fixed")   return SizePolicy::Fixed;
        if (name == "Fill")    return SizePolicy::Fill;
        if (name == "Fit")     return SizePolicy::Fit;
        if (name == "Stretch") return SizePolicy::Stretch;
        return SizePolicy::Fixed;
    }

    // Преобразование Unit в строку.
    [[nodiscard]] static std::string unitToName(Unit unit) {
        switch (unit) {
            case Unit::Pixel:   return "px";
            case Unit::Percent: return "%";
            case Unit::Auto:    return "Auto";
        }
        return "Auto";
    }

    // Преобразование SizePolicy в строку.
    [[nodiscard]] static std::string policyToName(SizePolicy policy) {
        switch (policy) {
            case SizePolicy::Fixed:   return "Fixed";
            case SizePolicy::Fill:    return "Fill";
            case SizePolicy::Fit:     return "Fit";
            case SizePolicy::Stretch: return "Stretch";
        }
        return "Fixed";
    }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    LayoutData  m_layoutData;   // локальная копия настроек

    // Универсальный вызов команды с передачей функций execute/undo для прямого изменения LayoutData.
    // Это временное решение, пока Widget::setProperty не поддерживает "widthUnit" и т.д.
    class LayoutPropertyCommand : public ICommand {
    public:
        using Action = std::function<void()>;
        LayoutPropertyCommand(Action exec, Action undo)
            : m_exec(std::move(exec)), m_undo(std::move(undo)) {}

        bool execute() override { m_exec(); return true; }
        bool undo()    override { m_undo(); return true; }
        std::string description() const override { return "Изменить геометрию"; }

    private:
        Action m_exec;
        Action m_undo;
    };

    void executeLayoutCommand(std::function<void()> exec, std::function<void()> undo) {
        auto cmd = std::make_unique<LayoutPropertyCommand>(std::move(exec), std::move(undo));
        m_doc.history().execute(std::move(cmd));
        m_doc.setModified(true);

        // После выполнения команды нужно обновить LayoutData в самом виджете,
        // чтобы изменения отразились в дереве и индексе.
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (widget) {
            widget->layoutData() = m_layoutData;
        }
    }

    // Изменение свойства через стандартную ChangePropertyCommand.
    void changeProperty(const std::string& name, const StateValue& value) {
        auto cmd = std::make_unique<ChangePropertyCommand>(m_doc, m_widgetId, name, value);
        m_doc.history().execute(std::move(cmd));
        m_doc.setModified(true);

        // Обновляем локальную копию после успешного выполнения.
        Widget* widget = m_doc.widgetTree().find(m_widgetId);
        if (widget) {
            m_layoutData = widget->layoutData();
        }
    }
};

} // namespace MirUI