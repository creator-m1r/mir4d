
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Color/ColorPalette.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Commands/ChangePropertyCommand.hpp"
#include <memory>
#include <string>
#include <functional>

namespace MirUI {

class ColorTokenEditor {
public:

    ColorTokenEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {

        m_currentColor = getTokenColor();
    }

    [[nodiscard]] Color currentColor() const { return m_currentColor; }

    void setColor(const Color& newColor) {
        if (newColor == m_currentColor) return;

        auto cmd = std::make_unique<ThemeColorCommand>(*this, m_currentColor, newColor);
        m_doc.history().execute(std::move(cmd));

    }

    bool showColorDialog() {

        static int step = 0;
        Color testColors[] = {
            Color::rgb(0.2f, 0.5f, 1.0f),
            Color::rgb(1.0f, 0.3f, 0.3f),
            Color::rgb(0.2f, 0.8f, 0.3f),
            Color::rgb(1.0f, 0.7f, 0.0f),
        };
        setColor(testColors[step % 4]);
        ++step;
        return true;
    }

    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

private:
    UIDocument& m_doc;
    std::string m_tokenName;
    Color m_currentColor;

    class ThemeColorCommand : public ICommand {
    public:
        ThemeColorCommand(ColorTokenEditor& editor, Color oldColor, Color newColor)
            : m_editor(editor)
            , m_oldColor(oldColor)
            , m_newColor(newColor)
        {}

        bool execute() override {

            m_editor.applyColor(m_newColor);
            m_editor.m_currentColor = m_newColor;
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {

            m_editor.applyColor(m_oldColor);
            m_editor.m_currentColor = m_oldColor;
            m_editor.m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override {
            return "Изменить цвет темы «" + m_editor.m_tokenName + "»";
        }

    private:
        ColorTokenEditor& m_editor;
        Color m_oldColor;
        Color m_newColor;
    };

    void applyColor(const Color& color) {
        Theme& theme = m_doc.themeManager().theme();
        ColorPalette& palette = theme.colors;

        using ColorMemberPtr = Color ColorPalette::*;
        static const std::unordered_map<std::string, ColorMemberPtr> tokenMap = {
            {"colors.background",      &ColorPalette::background},
            {"colors.surface",         &ColorPalette::surface},
            {"colors.surfaceHover",    &ColorPalette::surfaceHover},
            {"colors.surfaceActive",   &ColorPalette::surfaceActive},
            {"colors.textPrimary",     &ColorPalette::textPrimary},
            {"colors.textSecondary",   &ColorPalette::textSecondary},
            {"colors.textMuted",       &ColorPalette::textMuted},
            {"colors.accent",          &ColorPalette::accent},
            {"colors.accentHover",     &ColorPalette::accentHover},
            {"colors.accentActive",    &ColorPalette::accentActive},
            {"colors.border",          &ColorPalette::border},
            {"colors.error",           &ColorPalette::error},
            {"colors.warning",         &ColorPalette::warning},
            {"colors.success",         &ColorPalette::success}
        };

        auto it = tokenMap.find(m_tokenName);
        if (it != tokenMap.end()) {

            palette.*(it->second) = color;
        }

    }

    Color getTokenColor() const {
        const Theme& theme = m_doc.themeManager().theme();
        const ColorPalette& palette = theme.colors;

        using ColorMemberPtr = Color ColorPalette::*;
        static const std::unordered_map<std::string, ColorMemberPtr> tokenMap = {
            {"colors.background",      &ColorPalette::background},
            {"colors.surface",         &ColorPalette::surface},
            {"colors.surfaceHover",    &ColorPalette::surfaceHover},
            {"colors.surfaceActive",   &ColorPalette::surfaceActive},
            {"colors.textPrimary",     &ColorPalette::textPrimary},
            {"colors.textSecondary",   &ColorPalette::textSecondary},
            {"colors.textMuted",       &ColorPalette::textMuted},
            {"colors.accent",          &ColorPalette::accent},
            {"colors.accentHover",     &ColorPalette::accentHover},
            {"colors.accentActive",    &ColorPalette::accentActive},
            {"colors.border",          &ColorPalette::border},
            {"colors.error",           &ColorPalette::error},
            {"colors.warning",         &ColorPalette::warning},
            {"colors.success",         &ColorPalette::success}
        };

        auto it = tokenMap.find(m_tokenName);
        if (it != tokenMap.end()) {
            return palette.*(it->second);
        }
        return Color::transparent();
    }
};

}