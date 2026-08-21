
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/ThemeManager.hpp"
#include "../../Core/Theme/ThemeResolver.hpp"
#include "../../Core/Theme/WidgetStyle.hpp"
#include "../../Core/Theme/WidgetStateStyle.hpp"
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <algorithm>

namespace MirUI {

class WidgetStyleEditor {
public:

    WidgetStyleEditor(UIDocument& doc, WidgetType widgetType, WidgetState state = WidgetState::Normal)
        : m_doc(doc)
        , m_widgetType(widgetType)
        , m_state(state)
    {
        loadFromTheme();
    }

    void setWidgetType(WidgetType type) {
        if (type == m_widgetType) return;
        m_widgetType = type;
        loadFromTheme();
    }

    void setState(WidgetState state) {
        if (state == m_state) return;
        m_state = state;
        loadFromTheme();
    }

    [[nodiscard]] WidgetType widgetType() const { return m_widgetType; }
    [[nodiscard]] WidgetState widgetState() const { return m_state; }

    [[nodiscard]] Color  backgroundColor() const { return m_background; }
    [[nodiscard]] Color  foregroundColor() const { return m_foreground; }
    [[nodiscard]] Color  borderColor()     const { return m_border; }
    [[nodiscard]] double cornerRadius()    const { return m_cornerRadius; }
    [[nodiscard]] double opacity()         const { return m_opacity; }
    [[nodiscard]] bool   visible()         const { return m_visible; }

    [[nodiscard]] Font        font()          const { return m_font; }
    [[nodiscard]] std::string fontFamily()    const { return m_font.family; }
    [[nodiscard]] double      fontSize()      const { return m_font.size; }
    [[nodiscard]] FontWeight  fontWeight()    const { return m_font.weight; }
    [[nodiscard]] FontStyle   fontStyle()     const { return m_font.style; }

    [[nodiscard]] Color  shadowColor()   const { return m_shadow.color; }
    [[nodiscard]] double shadowOffsetX() const { return m_shadow.offsetX; }
    [[nodiscard]] double shadowOffsetY() const { return m_shadow.offsetY; }
    [[nodiscard]] double shadowBlur()    const { return m_shadow.blurRadius; }

    void setBackgroundColor(const Color& color) {
        if (color == m_background) return;
        executeEditCommand("Изменить цвет фона",
            [this, color]() { m_background = color; });
    }

    void setForegroundColor(const Color& color) {
        if (color == m_foreground) return;
        executeEditCommand("Изменить цвет текста",
            [this, color]() { m_foreground = color; });
    }

    void setBorderColor(const Color& color) {
        if (color == m_border) return;
        executeEditCommand("Изменить цвет рамки",
            [this, color]() { m_border = color; });
    }

    void setCornerRadius(double radius) {
        if (radius == m_cornerRadius) return;
        executeEditCommand("Изменить радиус скругления",
            [this, radius]() { m_cornerRadius = std::max(0.0, radius); });
    }

    void setOpacity(double opacity) {
        if (opacity == m_opacity) return;
        executeEditCommand("Изменить непрозрачность",
            [this, opacity]() { m_opacity = std::clamp(opacity, 0.0, 1.0); });
    }

    void setVisible(bool visible) {
        if (visible == m_visible) return;
        executeEditCommand(visible ? "Показать виджет" : "Скрыть виджет",
            [this, visible]() { m_visible = visible; });
    }

    void setFont(const Font& font) {
        if (font == m_font) return;
        executeEditCommand("Изменить шрифт",
            [this, font]() { m_font = font; });
    }

    void setFontFamily(const std::string& family) {
        if (family == m_font.family) return;
        executeEditCommand("Изменить семейство шрифта",
            [this, family]() { m_font.family = family; });
    }

    void setFontSize(double size) {
        if (size == m_font.size) return;
        executeEditCommand("Изменить размер шрифта",
            [this, size]() { m_font.size = std::max(1.0, size); });
    }

    void setFontWeight(FontWeight weight) {
        if (weight == m_font.weight) return;
        executeEditCommand("Изменить жирность шрифта",
            [this, weight]() { m_font.weight = weight; });
    }

    void setFontStyle(FontStyle style) {
        if (style == m_font.style) return;
        executeEditCommand("Изменить стиль шрифта",
            [this, style]() { m_font.style = style; });
    }

    void setShadowColor(const Color& color) {
        if (color == m_shadow.color) return;
        executeEditCommand("Изменить цвет тени",
            [this, color]() { m_shadow.color = color; });
    }

    void setShadowOffsetX(double offset) {
        if (offset == m_shadow.offsetX) return;
        executeEditCommand("Изменить смещение тени X",
            [this, offset]() { m_shadow.offsetX = offset; });
    }

    void setShadowOffsetY(double offset) {
        if (offset == m_shadow.offsetY) return;
        executeEditCommand("Изменить смещение тени Y",
            [this, offset]() { m_shadow.offsetY = offset; });
    }

    void setShadowBlur(double blur) {
        if (blur == m_shadow.blurRadius) return;
        executeEditCommand("Изменить размытие тени",
            [this, blur]() { m_shadow.blurRadius = std::max(0.0, blur); });
    }

    void resetToDefault() {
        WidgetStyle defaultStyle;

        Theme current = m_doc.themeManager().current();
        ThemeResolver resolver(current);
        WidgetStyle resolved = resolver.resolve(m_widgetType, m_state);
        
        setBackgroundColor(resolved.background);
        setForegroundColor(resolved.foreground);
        setBorderColor(resolved.border);
        setCornerRadius(resolved.cornerRadius);
        setOpacity(resolved.opacity);
        setVisible(resolved.visible);
        setFont(resolved.font);
        setShadowColor(resolved.shadow.color);
        setShadowOffsetX(resolved.shadow.offsetX);
        setShadowOffsetY(resolved.shadow.offsetY);
        setShadowBlur(resolved.shadow.blurRadius);
    }

    [[nodiscard]] UIDocument& document() { return m_doc; }
    [[nodiscard]] const UIDocument& document() const { return m_doc; }

private:
    UIDocument& m_doc;
    WidgetType  m_widgetType;
    WidgetState m_state;

    Color  m_background   = Color::transparent();
    Color  m_foreground   = Color::black();
    Color  m_border       = Color::transparent();
    double m_cornerRadius = 0.0;
    double m_opacity      = 1.0;
    bool   m_visible      = true;

    Font       m_font;
    ShadowData m_shadow;

    void loadFromTheme() {
        Theme current = m_doc.themeManager().current();
        ThemeResolver resolver(current);
        WidgetStyle style = resolver.resolve(m_widgetType, m_state);

        m_background   = style.background;
        m_foreground   = style.foreground;
        m_border       = style.border;
        m_cornerRadius = style.cornerRadius;
        m_opacity      = style.opacity;
        m_visible      = style.visible;
        m_font         = style.font;
        m_shadow       = style.shadow;
    }

    void applyToTheme() {

        m_doc.setModified(true);

    }

    void executeEditCommand(const std::string& description, std::function<void()> applyNewValues) {

        auto oldState = captureState();

        applyNewValues();

        auto newState = captureState();

        restoreState(oldState);

        auto cmd = std::make_unique<WidgetStyleEditCommand>(*this, oldState, newState, description);
        m_doc.history().execute(std::move(cmd));
    }

    struct StateSnapshot {
        Color  background, foreground, border;
        double cornerRadius, opacity;
        bool   visible;
        Font   font;
        ShadowData shadow;
    };

    StateSnapshot captureState() const {
        return {m_background, m_foreground, m_border, m_cornerRadius,
                m_opacity, m_visible, m_font, m_shadow};
    }

    void restoreState(const StateSnapshot& s) {
        m_background   = s.background;
        m_foreground   = s.foreground;
        m_border       = s.border;
        m_cornerRadius = s.cornerRadius;
        m_opacity      = s.opacity;
        m_visible      = s.visible;
        m_font         = s.font;
        m_shadow       = s.shadow;
    }

    class WidgetStyleEditCommand : public ICommand {
    public:
        WidgetStyleEditCommand(WidgetStyleEditor& editor,
                               const StateSnapshot& oldState,
                               const StateSnapshot& newState,
                               std::string description)
            : m_editor(editor)
            , m_old(oldState)
            , m_new(newState)
            , m_desc(std::move(description))
        {}

        bool execute() override {
            m_editor.restoreState(m_new);
            m_editor.applyToTheme();
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            m_editor.restoreState(m_old);
            m_editor.applyToTheme();
            m_editor.m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override { return m_desc; }

    private:
        WidgetStyleEditor& m_editor;
        StateSnapshot m_old;
        StateSnapshot m_new;
        std::string   m_desc;
    };
};

}