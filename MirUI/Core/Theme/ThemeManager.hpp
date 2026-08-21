// MirUI/Core/Theme/ThemeManager.hpp
// 🎛️ Менеджер тем — отвечает за хранение, регистрацию и переключение тем.
//
// ThemeManager — это центральное место, где MirUI хранит все доступные темы
// и знает, какая из них активна прямо сейчас. Он НЕ содержит логику загрузки
// из файла (это делает UIProjectSerializer) и НЕ зависит от платформы.
//
// Основные возможности:
//   • Зарегистрировать тему (добавить в коллекцию).
//   • Переключить активную тему по ThemeID.
//   • Получить текущую активную тему.
//   • Сбросить на стандартную тему (mir.light).
//   • Уведомить подписчиков о смене темы (через колбэк).
//
// Важно: ThemeManager владеет только коллекцией тем и текущим ID.
// Сами объекты Theme — это просто данные (структуры), они копируются
// при установке. Это безопасно и не требует управления памятью.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Theme.hpp"        // структура Theme (только данные)
#include "ThemeID.hpp"      // строковый идентификатор темы
#include <unordered_map>
#include <functional>
#include <string>
#include <optional>

namespace MirUI {

class ThemeManager {
public:
    // ── Конструктор ──────────────────────────────────────────
    ThemeManager() {
        // Сразу регистрируем встроенную светлую тему как тему по умолчанию.
        Theme defaultLight = Theme::createLight();
        registerTheme(defaultLight);
        setTheme(defaultLight.id);
    }

    // ── Текущая тема ──────────────────────────────────────────
    // Возвращает копию активной темы. Если тем нет (что невозможно после
    // конструктора), возвращает пустую тему.
    [[nodiscard]] Theme current() const {
        auto it = m_themes.find(m_currentId);
        if (it != m_themes.end()) {
            return it->second;
        }
        // Запасной вариант — отдаём светлую тему, если активная не найдена.
        return Theme::createLight();
    }

    // Установить активную тему, передав сам объект Theme.
    // Тема автоматически регистрируется, если её id ещё не известен.
    void setTheme(const Theme& theme) {
        // Регистрируем тему (если уже есть — перезапишет).
        m_themes[theme.id] = theme;
        // Переключаемся на неё.
        m_currentId = theme.id;
        // Оповещаем всех слушателей о смене темы.
        notifyListeners(theme.id);
    }

    // Установить активную тему по её идентификатору.
    // Если тема с таким id не зарегистрирована, ничего не делает.
    void setTheme(const ThemeID& id) {
        auto it = m_themes.find(id);
        if (it != m_themes.end()) {
            m_currentId = id;
            notifyListeners(id);
        }
    }

    // ── Идентификатор текущей темы ────────────────────────────
    [[nodiscard]] ThemeID currentThemeID() const {
        return m_currentId;
    }

    // ── Регистрация темы ──────────────────────────────────────
    // Добавляет тему в коллекцию. Если тема с таким id уже существует,
    // она будет заменена новой.
    void registerTheme(const Theme& theme) {
        m_themes[theme.id] = theme;
    }

    // Проверяет, зарегистрирована ли тема с указанным id.
    [[nodiscard]] bool hasTheme(const ThemeID& id) const {
        return m_themes.find(id) != m_themes.end();
    }

    // ── Сброс к стандартной теме ──────────────────────────────
    // Восстанавливает встроенную светлую тему и делает её активной.
    void resetToDefault() {
        Theme defaultLight = Theme::createLight();
        registerTheme(defaultLight);
        setTheme(defaultLight.id);
    }

    // ── Подписка на смену темы ────────────────────────────────
    // Позволяет рендерерам (SwiftUI, WinUI) и другим компонентам
    // узнавать, что тема изменилась, и перерисовывать интерфейс.
    // Колбэк получает идентификатор новой темы.
    using ThemeChangeCallback = std::function<void(const ThemeID& newThemeId)>;

    void onThemeChanged(ThemeChangeCallback callback) {
        m_listeners.push_back(std::move(callback));
    }

private:
    // Все зарегистрированные темы (ключ — ThemeID, значение — Theme).
    std::unordered_map<ThemeID, Theme, std::hash<ThemeID>> m_themes;

    // Идентификатор текущей активной темы.
    ThemeID m_currentId;

    // Список колбэков, вызываемых при смене темы.
    std::vector<ThemeChangeCallback> m_listeners;

    // Оповещает всех подписчиков о том, что тема изменилась.
    void notifyListeners(const ThemeID& newId) {
        for (auto& callback : m_listeners) {
            if (callback) {
                callback(newId);
            }
        }
    }
};

} // namespace MirUI