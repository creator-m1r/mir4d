// MirUI/Designer/Themes/ThemeEditor.hpp
// 🎨 Главный редактор темы в MirUI Designer.
//
// Тема MirUI объединяет цвета, метрики (отступы, радиусы, размеры),
// шрифты (типографику) и настройки анимаций. ThemeEditor предоставляет
// единую точку доступа для изменения всех этих параметров.
//
// Вместо того чтобы каждый раз вручную создавать ColorTokenEditor
// или MetricsEditor, другие части дизайнера (например, панель "Тема")
// могут просто запросить у ThemeEditor нужный редактор по имени токена:
//   • "colors.background" → ColorTokenEditor
//   • "metrics.spacingM" → MetricsEditor
//   • "typography.title" → TypographyEditor (пока заглушка)
//
// ThemeEditor сам следит за тем, чтобы все изменения применялись
// к текущей теме документа и попадали в историю Undo/Redo.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "ColorTokenEditor.hpp"
#include "MetricsEditor.hpp"
// #include "TypographyEditor.hpp"  // будет добавлен позже
#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace MirUI {

class ThemeEditor {
public:
    // ── Конструктор ──────────────────────────────────────────
    explicit ThemeEditor(UIDocument& document)
        : m_doc(document)
    {}

    // ── Получить редактор цвета по имени токена ──────────────
    // Возвращает указатель на ColorTokenEditor, если токен с таким именем
    // существует в палитре. Иначе nullptr.
    // Редактор создаётся при первом запросе и кешируется.
    ColorTokenEditor* colorEditor(const std::string& tokenName) {
        // Проверяем, не создан ли уже редактор для этого токена.
        auto it = m_colorEditors.find(tokenName);
        if (it != m_colorEditors.end()) {
            return it->second.get();
        }

        // Проверяем, что это действительно цветовой токен.
        if (!isValidColorToken(tokenName)) {
            return nullptr;
        }

        // Создаём новый редактор и сохраняем в кеше.
        auto editor = std::make_unique<ColorTokenEditor>(m_doc, tokenName);
        ColorTokenEditor* ptr = editor.get();
        m_colorEditors[tokenName] = std::move(editor);
        return ptr;
    }

    // ── Получить редактор метрики по имени токена ────────────
    MetricsEditor* metricsEditor(const std::string& tokenName) {
        auto it = m_metricsEditors.find(tokenName);
        if (it != m_metricsEditors.end()) {
            return it->second.get();
        }

        if (!isValidMetricsToken(tokenName)) {
            return nullptr;
        }

        auto editor = std::make_unique<MetricsEditor>(m_doc, tokenName);
        MetricsEditor* ptr = editor.get();
        m_metricsEditors[tokenName] = std::move(editor);
        return ptr;
    }

    // ── Получить редактор типографики (заглушка) ─────────────
    // TypographyEditor* typographyEditor(const std::string& tokenName);
    // Будет реализован после создания TypographyEditor.

    // ── Список всех известных токенов ─────────────────────────
    // Возвращает полный список идентификаторов всех токенов темы
    // (цветов, метрик, шрифтов), чтобы панель редактора могла
    // построить интерфейс.
    [[nodiscard]] std::vector<std::string> allTokenNames() const {
        std::vector<std::string> tokens;

        // Цвета
        tokens.insert(tokens.end(), {
            "colors.background",
            "colors.surface",
            "colors.surfaceHover",
            "colors.surfaceActive",
            "colors.textPrimary",
            "colors.textSecondary",
            "colors.textMuted",
            "colors.accent",
            "colors.accentHover",
            "colors.accentActive",
            "colors.border",
            "colors.error",
            "colors.warning",
            "colors.success"
        });

        // Метрики
        tokens.insert(tokens.end(), {
            "metrics.spacingXS",
            "metrics.spacingS",
            "metrics.spacingM",
            "metrics.spacingL",
            "metrics.spacingXL",
            "metrics.radiusS",
            "metrics.radiusM",
            "metrics.radiusL",
            "metrics.borderWidth",
            "metrics.controlHeight",
            "metrics.toolbarHeight",
            "metrics.panelWidth"
        });

        // Шрифты (будут добавлены после реализации TypographyEditor)
        // tokens.push_back("typography.title");
        // tokens.push_back("typography.subtitle");
        // ...

        return tokens;
    }

    // ── Сброс всей темы к значениям по умолчанию ─────────────
    // Создаёт команду, которая запоминает старую тему и применяет дефолтную,
    // чтобы можно было отменить сброс через Ctrl+Z.
    void resetToDefault() {
        // Сохраняем текущую тему для возможности Undo.
        Theme oldTheme = m_doc.themeManager().current();

        // Создаём и выполняем команду сброса темы.
        auto cmd = std::make_unique<ResetThemeCommand>(m_doc, oldTheme);
        m_doc.history().execute(std::move(cmd));
    }

    // ── Очистка кеша редакторов (при смене документа) ────────
    void clearCache() {
        m_colorEditors.clear();
        m_metricsEditors.clear();
    }

private:
    UIDocument& m_doc;

    // Кеш созданных редакторов, чтобы не создавать их повторно.
    std::unordered_map<std::string, std::unique_ptr<ColorTokenEditor>> m_colorEditors;
    std::unordered_map<std::string, std::unique_ptr<MetricsEditor>>  m_metricsEditors;

    // ── Проверки валидности токенов ──────────────────────────
    static bool isValidColorToken(const std::string& name) {
        static const std::vector<std::string> valid = {
            "colors.background", "colors.surface", "colors.surfaceHover",
            "colors.surfaceActive", "colors.textPrimary", "colors.textSecondary",
            "colors.textMuted", "colors.accent", "colors.accentHover",
            "colors.accentActive", "colors.border", "colors.error",
            "colors.warning", "colors.success"
        };
        return std::find(valid.begin(), valid.end(), name) != valid.end();
    }

    static bool isValidMetricsToken(const std::string& name) {
        static const std::vector<std::string> valid = {
            "metrics.spacingXS", "metrics.spacingS", "metrics.spacingM",
            "metrics.spacingL", "metrics.spacingXL", "metrics.radiusS",
            "metrics.radiusM", "metrics.radiusL", "metrics.borderWidth",
            "metrics.controlHeight", "metrics.toolbarHeight", "metrics.panelWidth"
        };
        return std::find(valid.begin(), valid.end(), name) != valid.end();
    }

    // ── Команда сброса темы (полная замена Theme) ────────────
    class ResetThemeCommand : public ICommand {
    public:
        ResetThemeCommand(UIDocument& doc, const Theme& oldTheme)
            : m_doc(doc)
            , m_oldTheme(oldTheme)
        {}

        bool execute() override {
            // Сохраняем текущую тему перед сбросом (на случай, если сброс уже был)
            // но мы уже сохранили старую тему в конструкторе, так что просто применяем дефолт.
            m_doc.themeManager().resetToDefault();
            m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            // Восстанавливаем старую тему.
            m_doc.themeManager().setTheme(m_oldTheme);
            m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override {
            return "Сбросить тему к значениям по умолчанию";
        }

    private:
        UIDocument& m_doc;
        Theme m_oldTheme;
    };
};

} // namespace MirUI