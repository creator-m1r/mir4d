
#pragma once

#include <string_view>

namespace MirUI::ShadowToken {

inline constexpr std::string_view None     = "shadow.none";
inline constexpr std::string_view Subtle   = "shadow.subtle";
inline constexpr std::string_view Panel    = "shadow.panel";
inline constexpr std::string_view Floating = "shadow.floating";
inline constexpr std::string_view Modal    = "shadow.modal";
inline constexpr std::string_view Active   = "shadow.active";
inline constexpr std::string_view Focus    = "shadow.focus";

}