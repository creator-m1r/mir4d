
#pragma once

#include "ColorTokenEditor.hpp"
#include "MetricsEditor.hpp"

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace MirUI {

class ThemeEditor {
public:

    explicit ThemeEditor(UIDocument& document)
        : m_doc(document)
    {}

    ColorTokenEditor* colorEditor(const std::string& tokenName) {

        auto it = m_colorEditors.find(tokenName);
        if (it != m_colorEditors.end()) {
            return it->second.get();
        }

        if (!isValidColorToken(tokenName)) {
            return nullptr;
        }

        auto editor = std::make_unique<ColorTokenEditor>(m_doc, tokenName);
        ColorTokenEditor* ptr = editor.get();
        m_colorEditors[tokenName] = std::move(editor);
        return ptr;
    }

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

    [[nodiscard]] std::vector<std::string> allTokenNames() const {
        std::vector<std::string> tokens;

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

        return tokens;
    }

    void resetToDefault() {

        Theme oldTheme = m_doc.themeManager().theme();

        auto cmd = std::make_unique<ResetThemeCommand>(m_doc, oldTheme);
        m_doc.history().execute(std::move(cmd));
    }

    void clearCache() {
        m_colorEditors.clear();
        m_metricsEditors.clear();
    }

private:
    UIDocument& m_doc;

    std::unordered_map<std::string, std::unique_ptr<ColorTokenEditor>> m_colorEditors;
    std::unordered_map<std::string, std::unique_ptr<MetricsEditor>>  m_metricsEditors;

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

    class ResetThemeCommand : public ICommand {
    public:
        ResetThemeCommand(UIDocument& doc, const Theme& oldTheme)
            : m_doc(doc)
            , m_oldTheme(oldTheme)
        {}

        bool execute() override {

            m_doc.themeManager().resetToDefault();
            m_doc.setModified(true);
            return true;
        }

        bool undo() override {

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

}