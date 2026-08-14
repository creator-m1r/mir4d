// MirUI/Designer/Themes/MetricsEditor.hpp
// 📏 Редактор метрик темы — расстояний, радиусов, стандартных размеров.
//
// Позволяет изменять числовые значения дизайн-токенов, определённых
// в структуре Metrics (spacingXS..XL, radiusS..L, borderWidth,
// controlHeight, toolbarHeight, panelWidth). Каждое изменение создаёт
// команду в истории Undo/Redo и немедленно обновляет Theme в документе.
//
// Использует команду, аналогичную той, что в ColorTokenEditor,
// но для double-значений. В будущем команда будет заменена на
// универсальную ChangePropertyCommand, когда система свойств темы
// будет расширена.
//
// Чистый C++23, без платформенных зависимостей.

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
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ и имя токена метрики (например, "metrics.spacingM").
    MetricsEditor(UIDocument& doc, const std::string& tokenName)
        : m_doc(doc)
        , m_tokenName(tokenName)
    {
        // Загружаем текущее значение из темы.
        m_currentValue = getTokenValue();
    }

    // ── Текущее значение ─────────────────────────────────────
    [[nodiscard]] double currentValue() const { return m_currentValue; }

    // ── Установка нового значения ────────────────────────────
    // Создаёт команду изменения метрики темы и выполняет её.
    void setValue(double newValue) {
        if (newValue == m_currentValue) return;

        auto cmd = std::make_unique<ThemeMetricCommand>(*this, m_currentValue, newValue);
        m_doc.history().execute(std::move(cmd));
    }

    // ── Имя токена ───────────────────────────────────────────
    [[nodiscard]] const std::string& tokenName() const { return m_tokenName; }

private:
    UIDocument& m_doc;
    std::string m_tokenName;
    double m_currentValue;

    // ── Внутренняя команда изменения метрики темы ─────────────
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

    // ── Применить значение к теме ────────────────────────────
    void applyValue(double value) {
        Theme theme = m_doc.themeManager().current();
        Metrics& metrics = theme.metrics;

        // Таблица указателей на члены Metrics для двойных значений.
        // Все поля Metrics — double, поэтому тип указателя: double Metrics::*.
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

        // Сохраняем изменённую тему обратно в менеджер.
        m_doc.themeManager().registerTheme(theme);
        m_doc.themeManager().setTheme(theme.id);
    }

    // ── Получить текущее значение из темы ────────────────────
    [[nodiscard]] double getTokenValue() const {
        const Theme theme = m_doc.themeManager().current();
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

} // namespace MirUI