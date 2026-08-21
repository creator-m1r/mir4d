// MirUI/Core/Theme/ThemeChangeEvent.hpp
// 📡 Событие смены темы — оповещает все компоненты о том, что активная тема изменилась.
//
// Когда ThemeManager переключает тему (по выбору пользователя, при загрузке
// проекта или сбросе к стандартной), он проходит по списку подписчиков
// и вызывает их колбэки. ThemeChangeEvent — это простой объект, который
// хранит старый и новый идентификаторы темы, чтобы подписчики могли
// понять, что именно изменилось, и перерисовать свои элементы.
//
// Использование:
//   ThemeManager manager;
//   manager.onThemeChanged([](const ThemeChangeEvent& event) {
//       // event.oldThemeId  — какой была тема
//       // event.newThemeId  — какой стала
//       // Перерисовать UI...
//   });
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "ThemeID.hpp"

namespace MirUI {

struct ThemeChangeEvent {
    ThemeID oldThemeId;   // идентификатор темы до изменения
    ThemeID newThemeId;   // идентификатор темы после изменения
};

} // namespace MirUI