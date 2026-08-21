// MirUI/Designer/Inspector/PropertyEditor.hpp
// ✨ Универсальный редактор свойства для инспектора (PropertyGrid).
//
// InspectorModel собирает список PropertyEntry — записей о каждом свойстве
// выделенного виджета. PropertyEditor — это "фасад", который для каждой записи
// создаёт внутри себя нужный специализированный редактор:
//   • ColorEditor  — если свойство является цветом
//   • FontEditor   — если свойство является шрифтом
//   • EnumEditor   — если свойство может принимать значения из списка
//   • иначе просто хранит значение и позволяет менять его как строку/число/флаг.
//
// Благодаря PropertyEditor, InspectorView (графическая часть инспектора)
// может работать со всеми свойствами одинаково: просто запрашивать текущее
// значение, возможные варианты, и вызывать setValue() при изменении.
// А PropertyEditor уже сам решает, какую команду (ChangePropertyCommand)
// создать и как сериализовать/десериализовать значение.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "InspectorModel.hpp"  // PropertyEntry, PropertyEditorType
#include "ColorEditor.hpp"
#include "FontEditor.hpp"
#include "EnumEditor.hpp"
#include "../Document/UIDocument.hpp"
#include "../../Core/State/StateValue.hpp"
#include <memory>
#include <string>
#include <vector>

namespace MirUI {

class PropertyEditor {
public:
    // Конструктор: создаёт нужный внутренний редактор на основе PropertyEntry.
    PropertyEditor(UIDocument& doc, WidgetID widgetId, const PropertyEntry& entry)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_entry(entry)
    {
        // В зависимости от типа свойства создаём соответствующий специализированный редактор.
        switch (entry.editorType) {
        case PropertyEditorType::Color:
            m_colorEditor = std::make_unique<ColorEditor>(doc, widgetId, entry.name);
            break;
        case PropertyEditorType::Font:
            m_fontEditor = std::make_unique<FontEditor>(doc, widgetId, entry.name);
            break;
        case PropertyEditorType::Enum:
            m_enumEditor = std::make_unique<EnumEditor>(doc, widgetId, entry.name, entry.enumValues);
            break;
        case PropertyEditorType::String:
        case PropertyEditorType::Integer:
        case PropertyEditorType::Float:
        case PropertyEditorType::Boolean:
            // Для простых типов не нужен отдельный редактор — мы будем работать
            // напрямую через ChangePropertyCommand.
            break;
        }
    }

    // ── Текущее значение (для отображения в инспекторе) ─────
    // Возвращает строковое представление значения, подходящее для показа пользователю.
    [[nodiscard]] std::string displayValue() const {
        // Если есть специализированный редактор, получаем значение из него.
        if (m_colorEditor) {
            return m_colorEditor->color().toHex(); // например, "#FF0000"
        }
        if (m_fontEditor) {
            Font f = m_fontEditor->currentFont();
            return f.family + " " + std::to_string(static_cast<int>(f.size)) + "pt";
        }
        if (m_enumEditor) {
            return m_enumEditor->currentValue();
        }

        // Для обычных значений используем визитёр.
        return stateValueToString(m_entry.currentValue);
    }

    // ── Тип редактора ────────────────────────────────────────
    [[nodiscard]] PropertyEditorType editorType() const { return m_entry.editorType; }

    // ── Список возможных значений (только для Enum) ──────────
    [[nodiscard]] const std::vector<std::string>& possibleValues() const {
        if (m_enumEditor) {
            return m_enumEditor->possibleValues();
        }
        static const std::vector<std::string> empty;
        return empty;
    }

    // ── Индекс текущего значения (для Enum в ComboBox) ──────
    [[nodiscard]] int currentIndex() const {
        if (m_enumEditor) {
            return m_enumEditor->currentIndex();
        }
        return -1;
    }

    // ── Показать специализированный диалог ───────────────────
    // Вызывается, когда пользователь нажимает кнопку "..." рядом с цветом или шрифтом.
    // Возвращает true, если диалог был подтверждён (значение изменилось).
    bool showDialog() {
        if (m_colorEditor) {
            return m_colorEditor->showColorDialog();
        }
        if (m_fontEditor) {
            return m_fontEditor->showFontDialog();
        }
        // Для Enum диалога нет — значение выбирается сразу из выпадающего списка.
        return false;
    }

    // ── Установка нового значения ────────────────────────────
    // Вызывается, когда пользователь изменил значение в поле ввода, переключателе
    // или выбрал элемент в выпадающем списке.
    // Создаёт ChangePropertyCommand и выполняет её через историю документа.
    void setValue(const StateValue& newValue) {
        // Если есть специализированный редактор, делегируем ему.
        if (m_colorEditor && std::holds_alternative<std::string>(newValue)) {
            // Ожидаем строку с HEX-цветом.
            m_colorEditor->setColor(Color::fromHex(std::get<std::string>(newValue)));
            return;
        }
        if (m_fontEditor && std::holds_alternative<std::string>(newValue)) {
            // Ожидаем строку с сериализованным шрифтом.
            // FontEditor::setFont принимает Font, но у нас есть статический десериализатор?
            // Проще: FontEditor внутри сам вызовет ChangePropertyCommand, если передать Font.
            // Здесь мы не можем создать Font из строки без парсинга, поэтому
            // для простоты пока просто создаём команду напрямую.
            break; // идём к универсальному созданию команды ниже
        }
        if (m_enumEditor && std::holds_alternative<std::string>(newValue)) {
            m_enumEditor->setValue(std::get<std::string>(newValue));
            return;
        }

        // Для всех остальных случаев создаём команду изменения свойства напрямую.
        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_entry.name, newValue
        );
        m_doc.history().execute(std::move(cmd));
    }

    // ── Установка значения из строки (для текстового поля) ───
    // Преобразует введённый текст в подходящий StateValue и вызывает setValue.
    void setValueFromString(const std::string& text) {
        switch (m_entry.editorType) {
        case PropertyEditorType::String:
            setValue(StateValue(text));
            break;
        case PropertyEditorType::Integer:
            try {
                setValue(StateValue(static_cast<int64_t>(std::stoll(text))));
            } catch (...) {}
            break;
        case PropertyEditorType::Float:
            try {
                setValue(StateValue(std::stod(text)));
            } catch (...) {}
            break;
        case PropertyEditorType::Boolean:
            // Для Boolean ожидается "true" или "false".
            if (text == "true" || text == "1") {
                setValue(StateValue(true));
            } else if (text == "false" || text == "0") {
                setValue(StateValue(false));
            }
            break;
        default:
            // Для Color, Font, Enum используем обычный setValue со строкой.
            setValue(StateValue(text));
            break;
        }
    }

    // ── Доступ к PropertyEntry ───────────────────────────────
    [[nodiscard]] const PropertyEntry& entry() const { return m_entry; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    PropertyEntry m_entry;

    // Специализированные редакторы (могут отсутствовать).
    std::unique_ptr<ColorEditor> m_colorEditor;
    std::unique_ptr<FontEditor>  m_fontEditor;
    std::unique_ptr<EnumEditor>  m_enumEditor;

    // ── Преобразование StateValue в строку для отображения ───
    static std::string stateValueToString(const StateValue& value) {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, bool>) {
                return v ? "Да" : "Нет";
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, double>) {
                return std::to_string(v);
            } else if constexpr (std::is_same_v<T, std::string>) {
                return v;
            }
        }, value);
    }
};

} // namespace MirUI