// MirUI/Workspace/WindowManager/WindowManager.hpp
// Менеджер окон – хранит описания окон (WindowDescriptor) и управляет их жизненным циклом.
// Не создаёт реальных платформенных окон! Только хранит информацию о том,
// какие окна должны существовать, и предоставляет методы для их активации/закрытия.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "WindowDescriptor.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <optional>
#include <stdexcept>

namespace MirUI {

class WindowManager {
public:
    // ── Создание и закрытие окон ─────────────────────────────

    // Объявить новое окно с заданными характеристиками.
    // Если окно с таким id уже существует, оно будет заменено.
    void createWindow(const WindowDescriptor& descriptor) {
        // Ищем, нет ли уже окна с таким же id.
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == descriptor.id; });
        if (it != m_windows.end()) {
            *it = descriptor; // обновляем существующее
        } else {
            m_windows.push_back(descriptor);
        }

        // Если это первое окно, делаем его активным автоматически.
        if (m_windows.size() == 1) {
            m_activeWindow = descriptor.id;
        }
    }

    // Закрыть (удалить) окно по его идентификатору.
    // Если это окно было активно, активным становится последнее оставшееся (если есть).
    void closeWindow(const std::string& id) {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        if (it == m_windows.end()) {
            return; // окно не найдено – ничего не делаем
        }

        // Удаляем из списка.
        m_windows.erase(it);

        // Если закрытое окно было активным, сбрасываем активное.
        if (m_activeWindow == id) {
            if (!m_windows.empty()) {
                m_activeWindow = m_windows.front().id;
            } else {
                m_activeWindow.reset();
            }
        }
    }

    // ── Активация ────────────────────────────────────────────

    // Сделать указанное окно активным (получающим фокус ввода).
    // Если окно не найдено – выбрасывает исключение.
    void activateWindow(const std::string& id) {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        if (it == m_windows.end()) {
            throw std::runtime_error("WindowManager::activateWindow: окно с id '" + id + "' не найдено.");
        }
        m_activeWindow = id;
    }

    // Получить идентификатор текущего активного окна.
    // Возвращает std::nullopt, если окон нет.
    [[nodiscard]] std::optional<std::string> activeWindow() const {
        return m_activeWindow;
    }

    // ── Доступ к описаниям ───────────────────────────────────

    // Найти окно по id и вернуть указатель на его описание (или nullptr).
    [[nodiscard]] WindowDescriptor* find(const std::string& id) {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        return (it != m_windows.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] const WindowDescriptor* find(const std::string& id) const {
        auto it = std::find_if(m_windows.begin(), m_windows.end(),
            [&](const WindowDescriptor& w) { return w.id == id; });
        return (it != m_windows.end()) ? &(*it) : nullptr;
    }

    // Получить список всех зарегистрированных окон.
    [[nodiscard]] const std::vector<WindowDescriptor>& windows() const {
        return m_windows;
    }

private:
    std::vector<WindowDescriptor> m_windows;    // Все описания окон.
    std::optional<std::string> m_activeWindow;  // ID активного окна (если есть).
};

} // namespace MirUI