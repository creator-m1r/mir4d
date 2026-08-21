
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Foundation/Typography/Font.hpp"
#include "../../Foundation/Typography/Typography.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace MirUI {

class TypographyEditor {
public:

    TypographyEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {

        m_currentFont = getTokenFont();
    }

    [[nodiscard]] Font currentFont() const { return m_currentFont; }

    void setFont(const Font& newFont) {
        if (newFont == m_currentFont) return;

        auto cmd = std::make_unique<ThemeTypographyCommand>(*this, m_currentFont, newFont);
        m_doc.history().execute(std::move(cmd));

    }

    bool showFontDialog() {
        static int step = 0;
        Font testFonts[] = {
            Font("System", 24.0, FontWeight::Bold, FontStyle::Normal),
            Font("System", 18.0, FontWeight::Medium, FontStyle::Italic),
            Font("Menlo", 14.0, FontWeight::Regular, FontStyle::Normal),
            Font("Georgia", 20.0, FontWeight::SemiBold, FontStyle::Normal)
        };
        setFont(testFonts[step % 4]);
        ++step;
        return true;
    }

    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

private:
    UIDocument& m_doc;
    std::string m_tokenName;
    Font m_currentFont;

    class ThemeTypographyCommand : public ICommand {
    public:
        ThemeTypographyCommand(TypographyEditor& editor, Font oldFont, Font newFont)
            : m_editor(editor)
            , m_oldFont(oldFont)
            , m_newFont(newFont)
        {}

        bool execute() override {
            m_editor.applyFont(m_newFont);
            m_editor.m_currentFont = m_newFont;
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            m_editor.applyFont(m_oldFont);
            m_editor.m_currentFont = m_oldFont;
            m_editor.m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override {
            return "Изменить шрифт темы «" + m_editor.m_tokenName + "»";
        }

    private:
        TypographyEditor& m_editor;
        Font m_oldFont;
        Font m_newFont;
    };

    void applyFont(const Font& font) {
        Theme& theme = m_doc.themeManager().theme();
        Typography& typography = theme.typography;

        using TypographyMemberPtr = Font Typography::*;
        static const std::unordered_map<std::string, TypographyMemberPtr> tokenMap = {
            {"typography.title",    &Typography::title},
            {"typography.subtitle", &Typography::subtitle},
            {"typography.body",     &Typography::body},
            {"typography.caption",  &Typography::caption},
            {"typography.button",   &Typography::button},
            {"typography.code",     &Typography::code}
        };

        auto it = tokenMap.find(m_tokenName);
        if (it != tokenMap.end()) {
            typography.*(it->second) = font;
        }
    }

    [[nodiscard]] Font getTokenFont() const {
        const Theme& theme = m_doc.themeManager().theme();
        const Typography& typography = theme.typography;

        using TypographyMemberPtr = Font Typography::*;
        static const std::unordered_map<std::string, TypographyMemberPtr> tokenMap = {
            {"typography.title",    &Typography::title},
            {"typography.subtitle", &Typography::subtitle},
            {"typography.body",     &Typography::body},
            {"typography.caption",  &Typography::caption},
            {"typography.button",   &Typography::button},
            {"typography.code",     &Typography::code}
        };

        auto it = tokenMap.find(m_tokenName);
        if (it != tokenMap.end()) {
            return typography.*(it->second);
        }
        return Font();
    }
};

}