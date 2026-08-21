
#pragma once

#include <string_view>

namespace MirUI::ColorToken {

inline constexpr std::string_view Background          = "interface.background";
inline constexpr std::string_view Surface             = "interface.surface";
inline constexpr std::string_view SurfaceHover        = "interface.surfaceHover";
inline constexpr std::string_view SurfaceActive       = "interface.surfaceActive";

inline constexpr std::string_view TextPrimary         = "interface.textPrimary";
inline constexpr std::string_view TextSecondary       = "interface.textSecondary";
inline constexpr std::string_view TextMuted           = "interface.textMuted";

inline constexpr std::string_view Accent              = "interface.accent";
inline constexpr std::string_view AccentHover         = "interface.accentHover";
inline constexpr std::string_view AccentActive        = "interface.accentActive";

inline constexpr std::string_view Border              = "interface.border";
inline constexpr std::string_view Separator           = "interface.separator";

inline constexpr std::string_view Error               = "status.error";
inline constexpr std::string_view ErrorText           = "status.errorText";
inline constexpr std::string_view Warning             = "status.warning";
inline constexpr std::string_view WarningText         = "status.warningText";
inline constexpr std::string_view Success             = "status.success";
inline constexpr std::string_view SuccessText         = "status.successText";

inline constexpr std::string_view Hover               = "state.hover";
inline constexpr std::string_view Press               = "state.press";
inline constexpr std::string_view Focus               = "state.focus";
inline constexpr std::string_view FocusRing           = "state.focusRing";
inline constexpr std::string_view Disabled            = "state.disabled";
inline constexpr std::string_view DisabledText        = "state.disabledText";
inline constexpr std::string_view Selected            = "state.selected";
inline constexpr std::string_view SelectedText        = "state.selectedText";

inline constexpr std::string_view ViewportBackground  = "viewport.background";
inline constexpr std::string_view ViewportGrid        = "viewport.grid";
inline constexpr std::string_view ViewportGridMajor   = "viewport.gridMajor";
inline constexpr std::string_view ViewportGridMinor   = "viewport.gridMinor";
inline constexpr std::string_view ViewportAxisX       = "viewport.axisX";
inline constexpr std::string_view ViewportAxisY       = "viewport.axisY";
inline constexpr std::string_view ViewportAxisZ       = "viewport.axisZ";
inline constexpr std::string_view ViewportSelected    = "viewport.selected";
inline constexpr std::string_view ViewportPreselected = "viewport.preselected";
inline constexpr std::string_view ViewportConstruction= "viewport.construction";
inline constexpr std::string_view ViewportHidden      = "viewport.hidden";
inline constexpr std::string_view ViewportDimension   = "viewport.dimension";
inline constexpr std::string_view ViewportSection     = "viewport.section";
inline constexpr std::string_view ViewportSnap        = "viewport.snap";
inline constexpr std::string_view ViewportOrigin      = "viewport.origin";
inline constexpr std::string_view ViewportGizmo       = "viewport.gizmo";
inline constexpr std::string_view ViewportGizmoX      = "viewport.gizmoX";
inline constexpr std::string_view ViewportGizmoY      = "viewport.gizmoY";
inline constexpr std::string_view ViewportGizmoZ      = "viewport.gizmoZ";

inline constexpr std::string_view InputBackground     = "input.background";
inline constexpr std::string_view InputBorder         = "input.border";
inline constexpr std::string_view InputPlaceholder    = "input.placeholder";

inline constexpr std::string_view Transparent         = "special.transparent";
inline constexpr std::string_view Inherit             = "special.inherit";

}