// MirUI/Core/Theme/ThemeResolver.hpp
// 🧩 Преобразователь темы — с каскадным наследованием стилей.
//    Для каждого виджета (тип + состояние + родительский контейнер)
//    возвращает полностью заполненный WidgetStyle.
//
// Каскадное наследование:
//   • Если виджет не задаёт какое-либо свойство (transparent фон, нулевой шрифт…),
//     значение автоматически берётся из родительского контейнера (по карте наследования).
//   • Если родитель тоже не задаёт – используется глобальный базовый стиль темы.
//
// Пример:
//   resolver.resolve(WidgetType::Button, WidgetState::Normal, WidgetType::Toolbar);
//   → кнопка унаследует фон и шрифт тулбара.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Theme.hpp"
#include "WidgetStyle.hpp"
#include "WidgetStateStyle.hpp"
#include "../Widget/WidgetType.hpp"
#include <unordered_map>

namespace MirUI {

class ThemeResolver {
public:
    // ── Конструктор ──────────────────────────────────────────
    explicit ThemeResolver(const Theme& theme)
        : m_theme(theme)   // копируем тему внутрь
    {
        buildInheritanceMap();
    }

    // ── Главный метод: получить стиль с учётом родительского контекста ──
    //   type           — тип виджета (Button, Label, …)
    //   state          — состояние (Normal, Hover, …)
    //   parentContext  — тип родительского контейнера (по умолчанию Window)
    [[nodiscard]] WidgetStyle resolve(WidgetType type,
                                      WidgetState state = WidgetState::Normal,
                                      WidgetType parentContext = WidgetType::Window) const {
        WidgetStyle base       = getBaseStyle(type, state);
        WidgetStyle parentStyle = getBaseStyle(parentContext, WidgetState::Normal);
        return cascade(base, parentStyle);
    }

    // ── Удобный метод: стиль по умолчанию (normal, контекст Window) ──
    [[nodiscard]] WidgetStyle resolveNormal(WidgetType type) const {
        return resolve(type, WidgetState::Normal, WidgetType::Window);
    }

    // ── Смена темы «на лету» ─────────────────────────────────
    void setTheme(const Theme& theme) {
        m_theme = theme;
    }
    [[nodiscard]] const Theme& theme() const { return m_theme; }

private:
    Theme m_theme;   // храним копию темы, чтобы можно было обновлять
    std::unordered_map<WidgetType, WidgetType> m_inheritanceMap;

    // ── Карта наследования: для каждого типа виджета — его типичный контейнер ──
    void buildInheritanceMap() {
        m_inheritanceMap[WidgetType::Button]       = WidgetType::Toolbar;
        m_inheritanceMap[WidgetType::Label]        = WidgetType::Panel;
        m_inheritanceMap[WidgetType::CheckBox]     = WidgetType::Panel;
        m_inheritanceMap[WidgetType::TextField]    = WidgetType::Panel;
        m_inheritanceMap[WidgetType::ComboBox]     = WidgetType::Panel;
        m_inheritanceMap[WidgetType::Slider]       = WidgetType::Panel;
        m_inheritanceMap[WidgetType::RadioButton]  = WidgetType::Panel;
        m_inheritanceMap[WidgetType::ProgressBar]  = WidgetType::Panel;
        m_inheritanceMap[WidgetType::Image]        = WidgetType::Panel;
        m_inheritanceMap[WidgetType::TableView]    = WidgetType::Panel;
        m_inheritanceMap[WidgetType::Toolbar]      = WidgetType::Window;
        m_inheritanceMap[WidgetType::Panel]        = WidgetType::Window;
        m_inheritanceMap[WidgetType::DockPanel]    = WidgetType::Window;
        m_inheritanceMap[WidgetType::ScrollView]   = WidgetType::Window;
        m_inheritanceMap[WidgetType::TabView]      = WidgetType::Window;
        m_inheritanceMap[WidgetType::Tree]         = WidgetType::Panel;
        m_inheritanceMap[WidgetType::PropertyGrid] = WidgetType::Panel;
        m_inheritanceMap[WidgetType::Viewport]     = WidgetType::Window;
        m_inheritanceMap[WidgetType::Window]       = WidgetType::Window;
    }

    // ── Базовый стиль типа с учётом состояния (без наследования) ──
    [[nodiscard]] WidgetStyle getBaseStyle(WidgetType type, WidgetState state) const {
        WidgetStyle style;

        // Общие значения из палитры темы
        style.background   = m_theme.colors.surface;
        style.foreground   = m_theme.colors.textPrimary;
        style.border       = m_theme.colors.border;
        style.cornerRadius = m_theme.metrics.radiusM;
        style.font         = m_theme.typography.body;
        style.opacity      = 1.0;

        // Специфичные значения для типов
        switch (type) {
            case WidgetType::Button:
                style.background   = m_theme.colors.accent;
                style.foreground   = Color::white();
                style.cornerRadius = m_theme.metrics.radiusM;
                style.font         = m_theme.typography.button;
                break;
            case WidgetType::Label:
                style.background   = Color::transparent();   // будет унаследован
                style.foreground   = m_theme.colors.textPrimary;
                break;
            case WidgetType::TextField:
                style.background   = m_theme.colors.inputBackground;
                style.border       = m_theme.colors.inputBorder;
                style.font         = m_theme.typography.body;
                break;
            case WidgetType::Toolbar:
                style.background   = m_theme.colors.surface;
                style.border       = Color::transparent();
                break;
            case WidgetType::Panel:
            case WidgetType::DockPanel:
                style.background   = m_theme.colors.surface;
                style.cornerRadius = m_theme.metrics.radiusL;
                break;
            case WidgetType::Viewport:
                style.background   = m_theme.colors.viewportBackground;
                break;
            default:
                break;
        }

        // Применяем состояние (hover, pressed…)
        WidgetStateStyle stateOverride = getStateOverride(type, state);
        return mergeWithState(style, stateOverride);
    }

    // ── Каскадное слияние: заполняем пустые поля из родительского стиля ──
    [[nodiscard]] WidgetStyle cascade(const WidgetStyle& childStyle,
                                      const WidgetStyle& parentStyle) const {
        WidgetStyle result = childStyle;

        // Фон
        if (result.background == Color::transparent()) {
            result.background = (parentStyle.background != Color::transparent())
                                ? parentStyle.background
                                : m_theme.colors.surface;
        }
        // Рамка
        if (result.border == Color::transparent()) {
            result.border = (parentStyle.border != Color::transparent())
                            ? parentStyle.border
                            : m_theme.colors.border;
        }
        // Скругление
        if (result.cornerRadius == 0.0 && parentStyle.cornerRadius > 0.0) {
            result.cornerRadius = parentStyle.cornerRadius;
        }
        // Шрифт (размер 0 означает «не задан»)
        if (result.font.size == 0.0) {
            result.font = (parentStyle.font.size > 0.0) ? parentStyle.font : m_theme.typography.body;
        }
        // Прозрачность
        if (result.opacity >= 1.0 && parentStyle.opacity < 1.0) {
            result.opacity = parentStyle.opacity;
        }
        // Тень
        if (result.shadow.color == Color::transparent()) {
            result.shadow = (parentStyle.shadow.color != Color::transparent())
                            ? parentStyle.shadow
                            : ShadowData{};
        }

        return result;
    }

    // ── Стиль состояния (hover, pressed, …) ──────────────────
    [[nodiscard]] WidgetStateStyle getStateOverride(WidgetType /*type*/, WidgetState state) const {
        WidgetStateStyle s;
        s.visible = true;

        switch (state) {
            case WidgetState::Normal:
                s.background = Color::transparent();
                s.foreground = Color::black();
                s.opacity    = 1.0;
                break;
            case WidgetState::Hover:
                s.background = m_theme.colors.hover;
                s.foreground = m_theme.colors.textPrimary;
                break;
            case WidgetState::Pressed:
                s.background = m_theme.colors.press;
                s.foreground = m_theme.colors.textPrimary;
                break;
            case WidgetState::Disabled:
                s.background = m_theme.colors.disabled;
                s.foreground = m_theme.colors.disabledText;
                s.opacity    = 0.6;
                break;
            case WidgetState::Focused:
                s.border = m_theme.colors.focusRing;
                break;
            case WidgetState::Selected:
                s.background = m_theme.colors.selected;
                s.foreground = m_theme.colors.selectedText;
                break;
        }
        return s;
    }

    // ── Применение состояния к базовому стилю ─────────────────
    [[nodiscard]] static WidgetStyle mergeWithState(const WidgetStyle& base,
                                                    const WidgetStateStyle& state) {
        WidgetStyle result = base;
        if (state.background != Color::transparent()) result.background = state.background;
        if (state.foreground != Color::black())       result.foreground = state.foreground;
        if (state.border != Color::transparent())     result.border = state.border;
        result.opacity = state.opacity;
        result.visible = state.visible;
        if (state.shadow.color != Color::transparent()) result.shadow = state.shadow;
        return result;
    }
};

} // namespace MirUI