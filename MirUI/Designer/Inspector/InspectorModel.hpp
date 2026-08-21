
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/State/StateValue.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include <vector>
#include <string>
#include <variant>
#include <optional>

namespace MirUI {

enum class PropertyEditorType {
    String,
    Integer,
    Float,
    Boolean,
    Color,
    Font,
    Enum
};

struct PropertyEntry {
    std::string name;
    std::string displayName;
    PropertyEditorType editorType;
    StateValue currentValue;
    std::vector<std::string> enumValues;
    bool readOnly = false;
};

class InspectorModel {
public:

    explicit InspectorModel(UIDocument& document)
        : m_doc(document)
    {}

    void inspectWidget(std::optional<WidgetID> widgetId) {
        m_currentWidgetId = widgetId;
        m_properties.clear();

        if (!widgetId.has_value()) {
            return;
        }

        Widget* widget = m_doc.widgetTree().find(*widgetId);
        if (!widget) return;

        const auto& allProps = widget->allProperties();

        for (const auto& [name, value] : allProps) {

            if (name == "id" || name == "type") continue;

            PropertyEntry entry;
            entry.name = name;
            entry.displayName = translatePropertyName(name);
            entry.currentValue = value;

            std::visit([&](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>) {
                    entry.editorType = PropertyEditorType::Boolean;
                } else if constexpr (std::is_same_v<T, int64_t>) {
                    entry.editorType = PropertyEditorType::Integer;
                } else if constexpr (std::is_same_v<T, double>) {
                    entry.editorType = PropertyEditorType::Float;
                } else if constexpr (std::is_same_v<T, std::string>) {

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

            entry.readOnly = isReadOnly(name, widget->type());

            m_properties.push_back(entry);
        }
    }

    void changeProperty(const std::string& name, const StateValue& newValue) {
        if (!m_currentWidgetId.has_value()) return;

        auto cmd = std::make_unique<ChangePropertyCommand>(
            m_doc, *m_currentWidgetId, name, newValue
        );
        m_doc.history().execute(std::move(cmd));

        inspectWidget(m_currentWidgetId);
    }

    [[nodiscard]] const std::vector<PropertyEntry>& properties() const {
        return m_properties;
    }

    [[nodiscard]] std::optional<WidgetID> currentWidgetId() const {
        return m_currentWidgetId;
    }

private:
    UIDocument& m_doc;
    std::optional<WidgetID> m_currentWidgetId;
    std::vector<PropertyEntry> m_properties;

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
        return name;
    }

    static bool isColorProperty(const std::string& , const std::string& value) {
        if (value.empty()) return false;
        if (value[0] != '#') return false;
        return (value.size() == 7 || value.size() == 9);
    }

    static bool isFontProperty(const std::string& , const std::string& value) {

        return value.find(';') != std::string::npos;
    }

    static bool isEnumProperty(const std::string& name, const StateValue& ) {

        return (name == "alignment");
    }

    static std::vector<std::string> getEnumValues(const std::string& name) {
        if (name == "alignment") {
            return {"Left", "Center", "Right"};
        }
        return {};
    }

    static bool isReadOnly(const std::string& , WidgetType ) {
        return false;
    }
};

}