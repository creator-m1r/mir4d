// MirUI/Designer/Themes/ColorTokenEditor.hpp
// 🎨 Редактор одного цветового токена темы.
//
// Тема MirUI (Theme) состоит из множества цветовых токенов:
// background, surface, textPrimary, accent, border, error и т.д.
// ColorTokenEditor позволяет выбрать один из этих токенов по имени
// и изменить его цвет через палитру (ColorEditor) или ввод HEX-значения.
// Все изменения автоматически применяются к текущей теме документа
// и отображаются в реальном времени через Renderer.
// Каждое изменение создаёт команду в истории Undo/Redo,
// поэтому можно откатить смену цвета точно так же, как любое другое действие.
//
// Важно: редактор работает напрямую с ThemeManager через UIDocument,
// временно используя собственный тип команды для изменения цвета темы.
// В будущем, когда система свойств документа будет расширена на тему,
// команда будет заменена на универсальную ChangePropertyCommand.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Foundation/Color/Color.hpp"
#include "../../Foundation/Color/ColorPalette.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include "../Commands/ChangePropertyCommand.hpp" // пока не используется, но будет
#include <memory>
#include <string>
#include <functional>

namespace MirUI {

class ColorTokenEditor {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ и имя цветового токена (например, "colors.accent").
    // Имена соответствуют полям структуры ColorPalette внутри Theme.
    ColorTokenEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {
        // Загружаем текущий цвет из темы.
        m_currentColor = getTokenColor();
    }

    // ── Текущий цвет токена ──────────────────────────────────
    [[nodiscard]] Color currentColor() const { return m_currentColor; }

    // ── Установка нового цвета ───────────────────────────────
    // Создаёт команду изменения цвета темы и выполняет её.
    void setColor(const Color& newColor) {
        if (newColor == m_currentColor) return;

        // Создаём и выполняем команду.
        auto cmd = std::make_unique<ThemeColorCommand>(*this, m_currentColor, newColor);
        m_doc.history().execute(std::move(cmd));
        // После выполнения команда вызовет applyColor(newColor) и обновит m_currentColor.
    }

    // ── Показать диалог выбора цвета ─────────────────────────
    // В будущем делегирует платформенному ColorPicker.
    // На этапе MVP просто меняет цвет на предопределённый для демонстрации.
    bool showColorDialog() {
        // Заглушка: циклически переключаем несколько цветов.
        static int step = 0;
        Color testColors[] = {
            Color::rgb(0.2f, 0.5f, 1.0f),  // синий
            Color::rgb(1.0f, 0.3f, 0.3f),  // красный
            Color::rgb(0.2f, 0.8f, 0.3f),  // зелёный
            Color::rgb(1.0f, 0.7f, 0.0f),  // оранжевый
        };
        setColor(testColors[step % 4]);
        ++step;
        return true; // диалог подтверждён
    }

    // ── Имя токена ───────────────────────────────────────────
    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

private:
    UIDocument& m_doc;
    std::string m_tokenName;
    Color m_currentColor;

    // ── Внутренняя команда для изменения цвета темы ──────────
    // Пока мы не имеем универсального механизма свойств темы,
    // используем специализированную команду.
    class ThemeColorCommand : public ICommand {
    public:
        ThemeColorCommand(ColorTokenEditor& editor, Color oldColor, Color newColor)
            : m_editor(editor)
            , m_oldColor(oldColor)
            , m_newColor(newColor)
        {}

        bool execute() override {
            // Применяем новый цвет к теме.
            m_editor.applyColor(m_newColor);
            m_editor.m_currentColor = m_newColor;
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            // Возвращаем старый цвет.
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

    // ── Применить цвет к теме ────────────────────────────────
    void applyColor(const Color& color) {
        Theme theme = m_doc.themeManager().current();
        ColorPalette& palette = theme.colors;

        // Сопоставляем имя токена с полем структуры ColorPalette.
        // Для этого используем таблицу указателей на члены (member pointers).
        // Можно было бы сделать через if-else, но для демонстрации современного C++ 
        // используем отображение строк на указатели.
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
            // Изменяем поле в палитре по указателю на член.
            palette.*(it->second) = color;
        }
        // Если имя не найдено, ничего не делаем (можно добавить вывод ошибки).

        // Сохраняем изменённую тему обратно в менеджер.
        m_doc.themeManager().registerTheme(theme);
        m_doc.themeManager().setTheme(theme.id);
    }

    // ── Получить текущий цвет токена из темы ─────────────────
    Color getTokenColor() const {
        const Theme theme = m_doc.themeManager().current();
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
        return Color::transparent(); // fallback
    }
};

} // namespace MirUI