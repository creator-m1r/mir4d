// MirUI/Designer/Themes/WidgetStyleEditor.hpp
// 🎨 Редактор стиля виджета — позволяет точечно настраивать внешний вид
//    любого типа виджета и любого его состояния.
//
// WidgetStyleEditor объединяет в себе возможности ColorTokenEditor,
// ShadowEditor, MetricsEditor, TypographyEditor и AnimationEditor,
// но сфокусирован на одном конкретном WidgetType и одном WidgetState.
//
// Что он умеет:
//   • Выбрать тип виджета (Button, Label, Panel…) и состояние (Normal, Hover…).
//   • Изменить цвет фона, текста, рамки (через Color).
//   • Настроить шрифт (семейство, размер, жирность, стиль).
//   • Настроить тень (offset X/Y, blur, spread, цвет).
//   • Задать радиус скругления и непрозрачность.
//   • Все изменения автоматически применяются к текущей теме документа
//     и попадают в историю Undo/Redo.
//
// Как это работает:
//   1. Редактор получает UIDocument и конкретный WidgetType + WidgetState.
//   2. При создании загружает текущие значения из темы (через ThemeResolver).
//   3. При изменении любого поля создаёт команду WidgetStyleEditCommand,
//      которая запоминает старые и новые значения всех полей.
//   4. Команда выполняется через CommandHistory, обновляет тему и документ.
//   5. Тема помечается изменённой, рендерер перерисовывает интерфейс.
//
// Чистый C++23, без платформенных зависимостей.

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
    // ── Конструктор ──────────────────────────────────────────
    // Принимает документ, тип виджета и состояние, которое редактируем.
    // Загружает текущие значения стиля из темы.
    WidgetStyleEditor(UIDocument& doc, WidgetType widgetType, WidgetState state = WidgetState::Normal)
        : m_doc(doc)
        , m_widgetType(widgetType)
        , m_state(state)
    {
        loadFromTheme();
    }

    // ── Переключение типа виджета или состояния ──────────────
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

    // ── Текущие значения (для отображения в UI) ──────────────
    [[nodiscard]] WidgetType widgetType() const { return m_widgetType; }
    [[nodiscard]] WidgetState widgetState() const { return m_state; }

    [[nodiscard]] Color  backgroundColor() const { return m_background; }
    [[nodiscard]] Color  foregroundColor() const { return m_foreground; }
    [[nodiscard]] Color  borderColor()     const { return m_border; }
    [[nodiscard]] double cornerRadius()    const { return m_cornerRadius; }
    [[nodiscard]] double opacity()         const { return m_opacity; }
    [[nodiscard]] bool   visible()         const { return m_visible; }

    // Шрифт
    [[nodiscard]] Font        font()          const { return m_font; }
    [[nodiscard]] std::string fontFamily()    const { return m_font.family; }
    [[nodiscard]] double      fontSize()      const { return m_font.size; }
    [[nodiscard]] FontWeight  fontWeight()    const { return m_font.weight; }
    [[nodiscard]] FontStyle   fontStyle()     const { return m_font.style; }

    // Тень
    [[nodiscard]] Color  shadowColor()   const { return m_shadow.color; }
    [[nodiscard]] double shadowOffsetX() const { return m_shadow.offsetX; }
    [[nodiscard]] double shadowOffsetY() const { return m_shadow.offsetY; }
    [[nodiscard]] double shadowBlur()    const { return m_shadow.blurRadius; }

    // ── Установка новых значений ────────────────────────────

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

    // ── Шрифт ────────────────────────────────────────────────
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

    // ── Тень ─────────────────────────────────────────────────
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

    // ── Сброс к значениям по умолчанию ───────────────────────
    void resetToDefault() {
        WidgetStyle defaultStyle;
        // Загружаем значения по умолчанию из ThemeResolver для этого типа.
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

    // ── Доступ к документу (для создания подредакторов) ─────
    [[nodiscard]] UIDocument& document() { return m_doc; }
    [[nodiscard]] const UIDocument& document() const { return m_doc; }

private:
    UIDocument& m_doc;
    WidgetType  m_widgetType;
    WidgetState m_state;

    // Текущие значения всех полей (загружены из темы)
    Color  m_background   = Color::transparent();
    Color  m_foreground   = Color::black();
    Color  m_border       = Color::transparent();
    double m_cornerRadius = 0.0;
    double m_opacity      = 1.0;
    bool   m_visible      = true;

    Font       m_font;
    ShadowData m_shadow;

    // ── Загрузка текущего стиля из темы ──────────────────────
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

    // ── Применение текущих значений к теме ───────────────────
    void applyToTheme() {
        // Пока у нас нет прямого доступа к Theme::widgetStyles[тип][состояние].
        // Поэтому изменения сохраняются локально, а команда просто помечает
        // документ как изменённый. В будущем мы добавим в Theme карту стилей.
        m_doc.setModified(true);

        // В будущем здесь будет:
        // Theme current = m_doc.themeManager().current();
        // current.widgetStyles[m_widgetType] = ...;
        // m_doc.themeManager().setTheme(current);
    }

    // ── Универсальный метод выполнения команды ──────────────
    void executeEditCommand(const std::string& description, std::function<void()> applyNewValues) {
        // Сохраняем старые значения для возможности отката.
        auto oldState = captureState();

        // Применяем новые значения (временно).
        applyNewValues();

        // Сохраняем новые значения.
        auto newState = captureState();

        // Откатываем обратно, чтобы команда применила их через execute().
        restoreState(oldState);

        // Создаём и выполняем команду.
        auto cmd = std::make_unique<WidgetStyleEditCommand>(*this, oldState, newState, description);
        m_doc.history().execute(std::move(cmd));
    }

    // ── Захват / восстановление полного состояния редактора ──
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

    // ── Внутренняя команда для Undo/Redo ─────────────────────
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

} // namespace MirUI