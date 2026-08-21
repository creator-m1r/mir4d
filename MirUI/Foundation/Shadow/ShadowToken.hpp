// MirUI/Foundation/Shadow/ShadowToken.hpp
// 🌑 Семантические токены теней — строковые ключи для всех теней в теме.
//
// Тени в интерфейсе создают ощущение глубины: панели «приподнимаются» над фоном,
// кнопки «вдавливаются» при нажатии, модальные окна «парят». Вместо того чтобы
// задавать параметры тени в каждом виджете отдельно, мы используем токены —
// осмысленные имена, за которыми закреплены конкретные наборы параметров.
//
// Токены разделены на уровни:
//   • none     — отсутствие тени (плоский вид).
//   • subtle   — едва заметная тень (для карточек).
//   • panel    — стандартная тень для панелей и контейнеров.
//   • floating — усиленная тень для плавающих элементов (меню, тултипы).
//   • modal    — глубокая тень для модальных окон.
//   • active   — тень для активных/нажатых элементов (вдавленный эффект).
//   • focus    — тень для элементов в фокусе (свечение).
//
// Чистый C++23, без платформенных зависимостей.

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

} // namespace MirUI::ShadowToken