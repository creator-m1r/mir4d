// MirUI/Foundation/Color/ColorPalette.hpp
// 🎨 Расширенная палитра цветов — теперь включает токены для CAD/Viewport.
//
// ColorPalette хранит значения цветов для ВСЕХ семантических токенов,
// определённых в ColorToken. Каждое поле — это конкретный цвет (Color),
// соответствующий определённому смысловому ключу.
//
// Теперь палитра разделена на логические группы:
//   • Interface   — фон, поверхности, текст, акцент, границы.
//   • Status      — ошибка, предупреждение, успех и их текстовые цвета.
//   • States      — hover, press, focus, disabled, selected.
//   • Input       — поля ввода, рамки, подсказки.
//   • Viewport    — всё, что связано с 3D/4D вьюпортом (сетка, оси, гизмо).
//
// Готовые статические методы (light, dark) создают полные палитры
// с разумными значениями по умолчанию.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Color.hpp"

namespace MirUI {

struct ColorPalette {
    // ── Интерфейс ────────────────────────────────────────────
    Color background;         // interface.background
    Color surface;            // interface.surface
    Color surfaceHover;       // interface.surfaceHover
    Color surfaceActive;      // interface.surfaceActive

    Color textPrimary;        // interface.textPrimary
    Color textSecondary;      // interface.textSecondary
    Color textMuted;          // interface.textMuted

    Color accent;             // interface.accent
    Color accentHover;        // interface.accentHover
    Color accentActive;       // interface.accentActive

    Color border;             // interface.border
    Color separator;          // interface.separator

    // ── Статус ──────────────────────────────────────────────
    Color error;              // status.error
    Color errorText;          // status.errorText
    Color warning;            // status.warning
    Color warningText;        // status.warningText
    Color success;            // status.success
    Color successText;        // status.successText

    // ── Состояния ───────────────────────────────────────────
    Color hover;              // state.hover
    Color press;              // state.press
    Color focus;              // state.focus
    Color focusRing;          // state.focusRing
    Color disabled;           // state.disabled
    Color disabledText;       // state.disabledText
    Color selected;           // state.selected
    Color selectedText;       // state.selectedText

    // ── Ввод ─────────────────────────────────────────────────
    Color inputBackground;    // input.background
    Color inputBorder;        // input.border
    Color inputPlaceholder;   // input.placeholder

    // ── Вьюпорт (3D / CAD) ──────────────────────────────────
    Color viewportBackground;   // viewport.background
    Color viewportGrid;         // viewport.grid
    Color viewportGridMajor;    // viewport.gridMajor
    Color viewportGridMinor;    // viewport.gridMinor
    Color viewportAxisX;        // viewport.axisX  (обычно красный)
    Color viewportAxisY;        // viewport.axisY  (обычно зелёный)
    Color viewportAxisZ;        // viewport.axisZ  (обычно синий)
    Color viewportSelected;     // viewport.selected
    Color viewportPreselected;  // viewport.preselected
    Color viewportConstruction; // viewport.construction (вспомогательная геометрия)
    Color viewportHidden;       // viewport.hidden (скрытые линии/грани)
    Color viewportDimension;    // viewport.dimension (размеры)
    Color viewportSection;      // viewport.section (сечения)
    Color viewportSnap;         // viewport.snap (привязка)
    Color viewportOrigin;       // viewport.origin (начало координат)
    Color viewportGizmo;        // viewport.gizmo (манипулятор)
    Color viewportGizmoX;       // viewport.gizmoX
    Color viewportGizmoY;       // viewport.gizmoY
    Color viewportGizmoZ;       // viewport.gizmoZ

    // ── Операторы сравнения ──────────────────────────────────
    bool operator==(const ColorPalette& other) const {
        return background == other.background &&
               surface == other.surface &&
               surfaceHover == other.surfaceHover &&
               surfaceActive == other.surfaceActive &&
               textPrimary == other.textPrimary &&
               textSecondary == other.textSecondary &&
               textMuted == other.textMuted &&
               accent == other.accent &&
               accentHover == other.accentHover &&
               accentActive == other.accentActive &&
               border == other.border &&
               separator == other.separator &&
               error == other.error &&
               errorText == other.errorText &&
               warning == other.warning &&
               warningText == other.warningText &&
               success == other.success &&
               successText == other.successText &&
               hover == other.hover &&
               press == other.press &&
               focus == other.focus &&
               focusRing == other.focusRing &&
               disabled == other.disabled &&
               disabledText == other.disabledText &&
               selected == other.selected &&
               selectedText == other.selectedText &&
               inputBackground == other.inputBackground &&
               inputBorder == other.inputBorder &&
               inputPlaceholder == other.inputPlaceholder &&
               viewportBackground == other.viewportBackground &&
               viewportGrid == other.viewportGrid &&
               viewportGridMajor == other.viewportGridMajor &&
               viewportGridMinor == other.viewportGridMinor &&
               viewportAxisX == other.viewportAxisX &&
               viewportAxisY == other.viewportAxisY &&
               viewportAxisZ == other.viewportAxisZ &&
               viewportSelected == other.viewportSelected &&
               viewportPreselected == other.viewportPreselected &&
               viewportConstruction == other.viewportConstruction &&
               viewportHidden == other.viewportHidden &&
               viewportDimension == other.viewportDimension &&
               viewportSection == other.viewportSection &&
               viewportSnap == other.viewportSnap &&
               viewportOrigin == other.viewportOrigin &&
               viewportGizmo == other.viewportGizmo &&
               viewportGizmoX == other.viewportGizmoX &&
               viewportGizmoY == other.viewportGizmoY &&
               viewportGizmoZ == other.viewportGizmoZ;
    }
    bool operator!=(const ColorPalette& other) const {
        return !(*this == other);
    }

    // ── Статические фабрики палитр ──────────────────────────

    // Стандартная светлая палитра (для интерфейса и CAD).
    static ColorPalette light() {
        ColorPalette p;

        // Интерфейс
        p.background    = Color::rgb(0.96f, 0.96f, 0.96f);
        p.surface       = Color::white();
        p.surfaceHover  = Color::rgb(0.95f, 0.95f, 0.95f);
        p.surfaceActive = Color::rgb(0.90f, 0.90f, 0.90f);
        p.textPrimary   = Color::rgb(0.10f, 0.10f, 0.10f);
        p.textSecondary = Color::rgb(0.40f, 0.40f, 0.40f);
        p.textMuted     = Color::rgb(0.60f, 0.60f, 0.60f);
        p.accent        = Color::rgb(0.00f, 0.48f, 1.00f);
        p.accentHover   = Color::rgb(0.00f, 0.40f, 0.85f);
        p.accentActive  = Color::rgb(0.00f, 0.32f, 0.70f);
        p.border        = Color::rgb(0.85f, 0.85f, 0.85f);
        p.separator     = Color::rgb(0.90f, 0.90f, 0.90f);

        // Статус
        p.error         = Color::rgb(0.90f, 0.20f, 0.20f);
        p.errorText     = Color::white();
        p.warning       = Color::rgb(1.00f, 0.70f, 0.00f);
        p.warningText   = Color::black();
        p.success       = Color::rgb(0.20f, 0.80f, 0.20f);
        p.successText   = Color::white();

        // Состояния
        p.hover         = Color::rgb(0.95f, 0.95f, 0.95f);
        p.press         = Color::rgb(0.90f, 0.90f, 0.90f);
        p.focus         = Color::rgba(0.00f, 0.48f, 1.00f, 0.3f);
        p.focusRing     = Color::rgb(0.00f, 0.48f, 1.00f);
        p.disabled      = Color::rgb(0.85f, 0.85f, 0.85f);
        p.disabledText  = Color::rgb(0.60f, 0.60f, 0.60f);
        p.selected      = Color::rgb(0.00f, 0.48f, 1.00f);
        p.selectedText  = Color::white();

        // Ввод
        p.inputBackground   = Color::white();
        p.inputBorder       = Color::rgb(0.80f, 0.80f, 0.80f);
        p.inputPlaceholder  = Color::rgb(0.70f, 0.70f, 0.70f);

        // Вьюпорт (CAD)
        p.viewportBackground   = Color::rgb(0.20f, 0.20f, 0.22f);
        p.viewportGrid         = Color::rgb(0.30f, 0.30f, 0.32f);
        p.viewportGridMajor    = Color::rgb(0.35f, 0.35f, 0.37f);
        p.viewportGridMinor    = Color::rgb(0.25f, 0.25f, 0.27f);
        p.viewportAxisX        = Color::rgb(1.00f, 0.20f, 0.20f);
        p.viewportAxisY        = Color::rgb(0.20f, 1.00f, 0.20f);
        p.viewportAxisZ        = Color::rgb(0.20f, 0.20f, 1.00f);
        p.viewportSelected     = Color::rgb(0.00f, 0.80f, 1.00f);
        p.viewportPreselected  = Color::rgb(0.50f, 0.90f, 1.00f);
        p.viewportConstruction = Color::rgb(0.50f, 0.50f, 0.50f);
        p.viewportHidden       = Color::rgb(0.40f, 0.40f, 0.40f);
        p.viewportDimension    = Color::rgb(1.00f, 0.80f, 0.00f);
        p.viewportSection      = Color::rgb(0.00f, 1.00f, 1.00f);
        p.viewportSnap         = Color::rgb(1.00f, 1.00f, 0.00f);
        p.viewportOrigin       = Color::white();
        p.viewportGizmo        = Color::white();
        p.viewportGizmoX       = Color::rgb(1.00f, 0.20f, 0.20f);
        p.viewportGizmoY       = Color::rgb(0.20f, 1.00f, 0.20f);
        p.viewportGizmoZ       = Color::rgb(0.20f, 0.20f, 1.00f);

        return p;
    }

    // Стандартная тёмная палитра.
    static ColorPalette dark() {
        ColorPalette p;

        // Интерфейс
        p.background    = Color::rgb(0.12f, 0.12f, 0.12f);
        p.surface       = Color::rgb(0.18f, 0.18f, 0.18f);
        p.surfaceHover  = Color::rgb(0.24f, 0.24f, 0.24f);
        p.surfaceActive = Color::rgb(0.30f, 0.30f, 0.30f);
        p.textPrimary   = Color::rgb(0.95f, 0.95f, 0.95f);
        p.textSecondary = Color::rgb(0.70f, 0.70f, 0.70f);
        p.textMuted     = Color::rgb(0.50f, 0.50f, 0.50f);
        p.accent        = Color::rgb(0.29f, 0.56f, 1.00f);
        p.accentHover   = Color::rgb(0.20f, 0.45f, 0.85f);
        p.accentActive  = Color::rgb(0.10f, 0.35f, 0.70f);
        p.border        = Color::rgb(0.30f, 0.30f, 0.30f);
        p.separator     = Color::rgb(0.25f, 0.25f, 0.25f);

        // Статус
        p.error         = Color::rgb(0.95f, 0.25f, 0.25f);
        p.errorText     = Color::white();
        p.warning       = Color::rgb(1.00f, 0.65f, 0.00f);
        p.warningText   = Color::black();
        p.success       = Color::rgb(0.25f, 0.85f, 0.25f);
        p.successText   = Color::white();

        // Состояния
        p.hover         = Color::rgb(0.24f, 0.24f, 0.24f);
        p.press         = Color::rgb(0.30f, 0.30f, 0.30f);
        p.focus         = Color::rgba(0.29f, 0.56f, 1.00f, 0.3f);
        p.focusRing     = Color::rgb(0.29f, 0.56f, 1.00f);
        p.disabled      = Color::rgb(0.20f, 0.20f, 0.20f);
        p.disabledText  = Color::rgb(0.50f, 0.50f, 0.50f);
        p.selected      = Color::rgb(0.29f, 0.56f, 1.00f);
        p.selectedText  = Color::white();

        // Ввод
        p.inputBackground   = Color::rgb(0.15f, 0.15f, 0.15f);
        p.inputBorder       = Color::rgb(0.35f, 0.35f, 0.35f);
        p.inputPlaceholder  = Color::rgb(0.50f, 0.50f, 0.50f);

        // Вьюпорт (CAD)
        p.viewportBackground   = Color::rgb(0.08f, 0.08f, 0.10f);
        p.viewportGrid         = Color::rgb(0.15f, 0.15f, 0.17f);
        p.viewportGridMajor    = Color::rgb(0.20f, 0.20f, 0.22f);
        p.viewportGridMinor    = Color::rgb(0.12f, 0.12f, 0.14f);
        p.viewportAxisX        = Color::rgb(1.00f, 0.30f, 0.30f);
        p.viewportAxisY        = Color::rgb(0.30f, 1.00f, 0.30f);
        p.viewportAxisZ        = Color::rgb(0.30f, 0.30f, 1.00f);
        p.viewportSelected     = Color::rgb(0.00f, 0.90f, 1.00f);
        p.viewportPreselected  = Color::rgb(0.40f, 0.80f, 1.00f);
        p.viewportConstruction = Color::rgb(0.60f, 0.60f, 0.60f);
        p.viewportHidden       = Color::rgb(0.30f, 0.30f, 0.30f);
        p.viewportDimension    = Color::rgb(1.00f, 0.85f, 0.00f);
        p.viewportSection      = Color::rgb(0.00f, 1.00f, 1.00f);
        p.viewportSnap         = Color::rgb(1.00f, 1.00f, 0.30f);
        p.viewportOrigin       = Color::white();
        p.viewportGizmo        = Color::white();
        p.viewportGizmoX       = Color::rgb(1.00f, 0.30f, 0.30f);
        p.viewportGizmoY       = Color::rgb(0.30f, 1.00f, 0.30f);
        p.viewportGizmoZ       = Color::rgb(0.30f, 0.30f, 1.00f);

        return p;
    }
};

} // namespace MirUI