
#pragma once

#include "../Document/UIDocument.hpp"
#include "../../Core/Theme/Theme.hpp"
#include "../../Foundation/Metrics/Metrics.hpp"
#include "../../Core/Commands/CommandHistory.hpp"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

namespace MirUI {

class MetricsEditor {
public:

    MetricsEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {

        m_currentValue = getTokenValue();
    }

    [[nodiscard]] double currentValue() const { return m_currentValue; }

    void setValue(double newValue) {
        if (newValue == m_currentValue) return;

        auto cmd = std::make_unique<ThemeMetricCommand>(*this, m_currentValue, newValue);
        m_doc.history().execute(std::move(cmd));
    }

    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

private:
    UIDocument& m_doc;
    std::string m_tokenName;
    double m_currentValue;

    class ThemeMetricCommand : public ICommand {
    public:
        ThemeMetricCommand(MetricsEditor& editor, double oldValue, double newValue)
            : m_editor(editor)
            , m_oldValue(oldValue)
            , m_newValue(newValue)
        {}

        bool execute() override {
            m_editor.applyValue(m_newValue);
            m_editor.m_currentValue = m_newValue;
            m_editor.m_doc.setModified(true);
            return true;
        }

        bool undo() override {
            m_editor.applyValue(m_oldValue);
            m_editor.m_currentValue = m_oldValue;
            m_editor.m_doc.setModified(true);
            return true;
        }

        [[nodiscard]] std::string description() const override {
            return "Изменить метрику «" + m_editor.m_tokenName + "»";
        }

    private:
        MetricsEditor& m_editor;
        double m_oldValue;
        double m_newValue;
    };

    void applyValue(double value) {
        Theme& theme = m_doc.themeManager().theme();
        Metrics& metrics = theme.metrics;

        using MetricMemberPtr = double Metrics::*;
        static const std::unordered_map<std::string, MetricMemberPtr> tokenMap = {
            {"metrics.spacingXS",   &Metrics::spacingXS},
            {"metrics.spacingS",    &Metrics::spacingS},
            {"metrics.spacingM",    &Metrics::spacingM},
            {"metrics.spacingL",    &Metrics::spacingL},
            {"metrics.spacingXL",   &Metrics::spacingXL},
            {"metrics.radiusS",     &Metrics::radiusS},
            {"metrics.radiusM",     &Metrics::radiusM},
            {"metrics.radiusL",     &Metrics::radiusL},
            {"metrics.borderWidth", &Metrics::borderWidth},
            {"metrics.controlHeight",&Metrics::controlHeight},
            {"metrics.toolbarHeight",&Metrics::toolbarHeight},
            {"metrics.panelWidth",  &Metrics::panelWidth}
        };

        auto it = tokenMap.find(m_tokenName);
        if (it != tokenMap.end()) {
            metrics.*(it->second) = value;
        }
    }

    [[nodiscard]] double getTokenValue() const {
        const Theme& theme = m_doc.themeManager().theme();
        const Metrics& metrics = theme.metrics;

        using MetricMemberPtr = double Metrics::*;
        static const std::unordered_map<std::string, MetricMemberPtr> tokenMap = {
            {"metrics.spacingXS",   &Metrics::spacingXS},
            {"metrics.spacingS",    &Metrics::spacingS},
            {"metrics.spacingM",    &Metrics::spacingM},
            {"metrics.spacingL",    &Metrics::spacingL},
            {"metrics.spacingXL",   &Metrics::spacingXL},
            {"metrics.radiusS",     &Metrics::radiusS},
            {"metrics.radiusM",     &Metrics::radiusM},
            {"metrics.radiusL",     &Metrics::radiusL},
            {"metrics.borderWidth", &Metrics::borderWidth},
            {"metrics.controlHeight",&Metrics::controlHeight},
            {"metrics.toolbarHeight",&Metrics::toolbarHeight},
            {"metrics.panelWidth",  &Metrics::panelWidth}
        };

        auto it = tokenMap.find(m_tokenName);
        if (it != tokenMap.end()) {
            return metrics.*(it->second);
        }
        return 0.0;
    }
};

}