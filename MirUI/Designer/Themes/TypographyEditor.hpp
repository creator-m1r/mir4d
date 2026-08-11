// MirUI/Designer/Themes/TypographyEditor.hpp
// 🔤 Редактор типографики темы — управляет шрифтовыми токенами.
//
// Тема MirUI содержит структуру Typography, в которой определены
// основные текстовые стили интерфейса: заголовок (title), подзаголовок
// (subtitle), основной текст (body), подпись (caption), кнопка (button),
// моноширинный (code). TypographyEditor позволяет выбрать один из этих
// стилей по имени и изменить его шрифт (семейство, размер, начертание, стиль).
// Изменения немедленно применяются к теме документа и попадают в историю
// Undo/Redo, как и любые другие действия в редакторе.
//
// Сам редактор не рисует шрифты — он только хранит состояние и выполняет
// команды. Отображение выбранного шрифта (название, размер, пример текста)
// будет делать платформенный рендерер.
//
// Чистый C++23, без платформенных зависимостей.

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
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ и имя токена типографики (например, "typography.title").
    TypographyEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {
        // Загружаем текущий шрифт из темы.
        m_currentFont = getTokenFont();
    }

    // ── Текущий шрифт токена ─────────────────────────────────
    [[nodiscard]] Font currentFont() const { return m_currentFont; }

    // ── Установка нового шрифта ──────────────────────────────
    // Создаёт команду изменения шрифта темы и выполняет её.
    void setFont(const Font& newFont) {
        if (newFont == m_currentFont) return;

        auto cmd = std::make_unique<ThemeTypographyCommand>(*this, m_currentFont, newFont);
        m_doc.history().execute(std::move(cmd));
        // После выполнения команда вызовет applyFont(newFont) и обновит m_currentFont.
    }

    // ── Показать диалог выбора шрифта ─────────────────────────
    // В будущем вызовет платформенный диалог шрифтов.
    // На этапе MVP просто переключает предопределённые шрифты для демонстрации.
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
        return true; // диалог подтверждён
    }

    // ── Имя токена ───────────────────────────────────────────
    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

private:
    UIDocument& m_doc;
    std::string m_tokenName;
    Font m_currentFont;

    // ── Внутренняя команда изменения шрифта темы ─────────────
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

    // ── Применить шрифт к теме ───────────────────────────────
    void applyFont(const Font& font) {
        Theme& theme = m_doc.themeManager().theme();
        Typography& typography = theme.typography;

        // Таблица указателей на члены Typography для шрифтов.
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

    // ── Получить текущий шрифт токена из темы ────────────────
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
        return Font(); // fallback
    }
};

} // namespace MirUI