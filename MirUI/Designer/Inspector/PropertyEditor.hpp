
#pragma once

#include "InspectorModel.hpp"
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

    PropertyEditor(UIDocument& doc, WidgetID widgetId, const PropertyEntry& entry)
        : m_doc(doc)
        , m_widgetId(widgetId)
        , m_entry(entry)
    {

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

            break;
        }
    }

    [[nodiscard]] std::string displayValue() const {

        if (m_colorEditor) {
            return m_colorEditor->color().toHex();
        }
        if (m_fontEditor) {
            Font f = m_fontEditor->currentFont();
            return f.family + " " + std::to_string(static_cast<int>(f.size)) + "pt";
        }
        if (m_enumEditor) {
            return m_enumEditor->currentValue();
        }

        return stateValueToString(m_entry.currentValue);
    }

    [[nodiscard]] PropertyEditorType editorType() const { return m_entry.editorType; }

    [[nodiscard]] const std::vector<std::string>& possibleValues() const {
        if (m_enumEditor) {
            return m_enumEditor->possibleValues();
        }
        static const std::vector<std::string> empty;
        return empty;
    }

    [[nodiscard]] int currentIndex() const {
        if (m_enumEditor) {
            return m_enumEditor->currentIndex();
        }
        return -1;
    }

    bool showDialog() {
        if (m_colorEditor) {
            return m_colorEditor->showColorDialog();
        }
        if (m_fontEditor) {
            return m_fontEditor->showFontDialog();
        }

        return false;
    }

    void setValue(const StateValue& newValue) {

        if (m_colorEditor && std::holds_alternative<std::string>(newValue)) {

            m_colorEditor->setColor(Color::fromHex(std::get<std::string>(newValue)));
            return;
        }
        if (m_fontEditor && std::holds_alternative<std::string>(newValue)) {

            break;
        }
        if (m_enumEditor && std::holds_alternative<std::string>(newValue)) {
            m_enumEditor->setValue(std::get<std::string>(newValue));
            return;
        }

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, m_widgetId, m_entry.name, newValue
        );
        m_doc.history().execute(std::move(cmd));
    }

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

            if (text == "true" || text == "1") {
                setValue(StateValue(true));
            } else if (text == "false" || text == "0") {
                setValue(StateValue(false));
            }
            break;
        default:

            setValue(StateValue(text));
            break;
        }
    }

    [[nodiscard]] const PropertyEntry& entry() const { return m_entry; }

private:
    UIDocument& m_doc;
    WidgetID    m_widgetId;
    PropertyEntry m_entry;

    std::unique_ptr<ColorEditor> m_colorEditor;
    std::unique_ptr<FontEditor>  m_fontEditor;
    std::unique_ptr<EnumEditor>  m_enumEditor;

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

}