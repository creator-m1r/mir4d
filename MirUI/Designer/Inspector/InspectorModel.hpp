// MirUI/Designer/Inspector/InspectorModel.hpp
// 🧰 Модель инспектора свойств — связывает выделенный виджет с редакторами.
//
// Когда ты щёлкаешь по кнопке на холсте, SelectionManager сообщает,
// что теперь выделен новый виджет. InspectorModel получает это событие,
// загружает все свойства виджета (через Widget::getProperty / allProperties),
// определяет, какого типа каждое свойство (цвет, шрифт, перечисление,
// строка, число, логическое), и готовит список PropertyEntry.
//
// Панель инспектора (InspectorView) читает этот список и для каждого
// свойства создаёт нужный редактор: ColorEditor, FontEditor, EnumEditor
// или универсальный редактор строки/числа. Когда пользователь изменяет
// значение, редактор вызывает InspectorModel::changeProperty, которая
// создаёт ChangePropertyCommand и выполняет её через историю документа.
//
// InspectorModel НЕ содержит визуальных элементов — только данные и логику.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include <vector>
#include <string>
#include <variant>
#include <optional>

namespace MirUI {

// ── Типы свойств, которые поддерживает инспектор ────────────
// Эти типы определяют, какой редактор будет показан.
enum class PropertyEditorType {
    String,       // текстовое поле
    Integer,      // целое число
    Float,        // дробное число
    Boolean,      // галочка (переключатель)
    Color,        // редактор цвета (ColorEditor)
    Font,         // редактор шрифта (FontEditor)
    Enum          // выпадающий список (EnumEditor)
};

// ── Запись о свойстве для отображения в инспекторе ─────────
struct PropertyEntry {
    std::string name;           // имя свойства (например, "text", "color")
    std::string displayName;    // отображаемое имя (например, "Текст", "Цвет фона")
    PropertyEditorType editorType; // какой редактор использовать
    StateValue currentValue;    // текущее значение
    std::vector<std::string> enumValues; // список значений для Enum (пустой для других типов)
    bool readOnly = false;      // можно ли редактировать
};

class InspectorModel {
public:
    // Конструктор принимает документ для доступа к виджетам и истории.
    explicit InspectorModel(UIDocument& document)
        : m_doc(document)
    {}

    // ── Обновление модели при смене выделенного виджета ──────
    // Вызывается, когда SelectionManager сообщает о новом выделении.
    // Загружает свойства выделенного виджета (или очищает список,
    // если ничего не выделено).
    void inspectWidget(std::optional<WidgetID> widgetId) {
        m_currentWidgetId = widgetId;
        m_properties.clear();

        if (!widgetId.has_value()) {
            return; // ничего не выделено — список пуст
        }

        Widget* widget = m_doc.widgetTree().find(*widgetId);
        if (!widget) return;

        // Получаем все свойства виджета (включая стандартные и пользовательские).
        const auto& allProps = widget->allProperties();

        // Для каждого свойства определяем, как его отображать.
        for (const auto& [name, value] : allProps) {
            // Пропускаем служебные свойства, которые не редактируются.
            if (name == "id" || name == "type") continue;

            PropertyEntry entry;
            entry.name = name;
            entry.displayName = translatePropertyName(name); // переводим в читаемое
            entry.currentValue = value;

            // Определяем тип редактора по значению.
            std::visit([&](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>) {
                    entry.editorType = PropertyEditorType::Boolean;
                } else if constexpr (std::is_same_v<T, int64_t>) {
                    entry.editorType = PropertyEditorType::Integer;
                } else if constexpr (std::is_same_v<T, double>) {
                    entry.editorType = PropertyEditorType::Float;
                } else if constexpr (std::is_same_v<T, std::string>) {
                    // Особая обработка для строк, которые содержат цвет или шрифт.
                    if (isColorProperty(name, std::get<std::string>(value))) {
                        entry.editorType = PropertyEditorType::Color;
                    } else if (isFontProperty(name, std::get<std::string>(value))) {
                        entry.editorType = PropertyEditorType::Font;
                    } else if (isEnumProperty(name, value)) {
                        entry.editorType = PropertyEditorType::Enum;
                        entry.enumValues = getEnumValues(name);
                    } else {
                        entry.editorType = PropertyEditorType::String;
                    }
                }
            }, value);

            // Некоторые свойства могут быть readOnly (например, имя у системных виджетов).
            entry.readOnly = isReadOnly(name, widget->type());

            m_properties.push_back(entry);
        }
    }

    // ── Изменение свойства ───────────────────────────────────
    // Вызывается редактором, когда пользователь изменил значение.
    // Создаёт ChangePropertyCommand и выполняет её (Undo/Redo).
    void changeProperty(const std::string& name, const StateValue& newValue) {
        if (!m_currentWidgetId.has_value()) return;

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, *m_currentWidgetId, name, newValue
        );
        m_doc.history().execute(std::move(cmd));

        // После выполнения команды перечитываем свойства, чтобы отразить изменения.
        inspectWidget(m_currentWidgetId);
    }

    // ── Доступ к списку свойств ──────────────────────────────
    [[nodiscard]] const std::vector<PropertyEntry>& properties() const {
        return m_properties;
    }

    // ── Текущий виджет ──────────────────────────────────────
    [[nodiscard]] std::optional<WidgetID> currentWidgetId() const {
        return m_currentWidgetId;
    }

private:
    UIDocument& m_doc;
    std::optional<WidgetID> m_currentWidgetId;
    std::vector<PropertyEntry> m_properties;

    // ── Вспомогательные методы для определения типа редактора ──

    // Переводит техническое имя свойства в читаемое.
    static std::string translatePropertyName(const std::string& name) {
        if (name == "name")     return "Имя";
        if (name == "visible")  return "Видимость";
        if (name == "enabled")  return "Доступность";
        if (name == "width")    return "Ширина";
        if (name == "height")   return "Высота";
        if (name == "minWidth") return "Мин. ширина";
        if (name == "minHeight")return "Мин. высота";
        if (name == "maxWidth") return "Макс. ширина";
        if (name == "maxHeight")return "Макс. высота";
        if (name == "text")     return "Текст";
        if (name == "color" || name == "background") return "Цвет фона";
        if (name == "font")     return "Шрифт";
        if (name == "alignment") return "Выравнивание";
        return name; // если перевода нет, возвращаем как есть
    }

    // Проверяет, является ли строковое значение цветом (начинается с # и длина 7 или 9).
    static bool isColorProperty(const std::string& /*name*/, const std::string& value) {
        if (value.empty()) return false;
        if (value[0] != '#') return false;
        return (value.size() == 7 || value.size() == 9); // #RRGGBB или #RRGGBBAA
    }

    // Проверяет, является ли строковое значение шрифтом (содержит ';').
    static bool isFontProperty(const std::string& /*name*/, const std::string& value) {
        // Шрифт хранится как "family;size;weight;style"
        return value.find(';') != std::string::npos;
    }

    // Проверяет, является ли свойство перечислением.
    // Пока мы знаем только "alignment", но можно расширить.
    static bool isEnumProperty(const std::string& name, const StateValue& /*value*/) {
        // Если свойство называется "alignment", это enum.
        return (name == "alignment");
    }

    // Возвращает возможные значения для enum-свойства.
    static std::vector<std::string> getEnumValues(const std::string& name) {
        if (name == "alignment") {
            return {"Left", "Center", "Right"};
        }
        return {};
    }

    // Проверяет, можно ли редактировать свойство.
    // Например, имя у корневого окна лучше не давать менять? Пока всё редактируемо.
    static bool isReadOnly(const std::string& /*name*/, WidgetType /*type*/) {
        return false; // пока всё редактируемо
    }
};

} // namespace MirUI