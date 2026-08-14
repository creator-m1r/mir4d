// MirUI/Workspace/WorkspaceManager/WorkspaceManager.hpp
// Менеджер рабочих пространств (WorkspaceManager).
// Хранит все созданные рабочие пространства, позволяет переключаться между ними,
// а также сохранять и загружать их конфигурацию.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Workspace.hpp"
#include <vector>
#include <string>
#include <optional>
#include <algorithm>
#include <stdexcept>

namespace MirUI {

class WorkspaceManager {
public:
    // ── Создание и удаление рабочих пространств ──────────────

    // Создать новое рабочее пространство и добавить его в список.
    // Если рабочее пространство с таким id уже существует, ничего не делает и возвращает false.
    // Иначе создаёт и возвращает true.
    bool create(const Workspace& workspace) {
        // Проверяем, нет ли уже workspace с таким же id.
        auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(),
            [&](const Workspace& ws) { return ws.id == workspace.id; });
        if (it != m_workspaces.end()) {
            // Уже существует — не создаём дубликат.
            return false;
        }

        // Добавляем новое рабочее пространство в конец списка.
        m_workspaces.push_back(workspace);

        // Если это самое первое добавленное рабочее пространство,
        // автоматически делаем его активным.
        if (m_workspaces.size() == 1) {
            m_activeWorkspace = workspace.id;
        }

        return true;
    }

    // Удалить рабочее пространство по его идентификатору.
    // Возвращает true, если удаление прошло успешно.
    bool remove(const std::string& id) {
        // Ищем рабочее пространство с заданным id.
        auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(),
            [&](const Workspace& ws) { return ws.id == id; });

        // Если не нашли — возвращаем false.
        if (it == m_workspaces.end()) {
            return false;
        }

        // Если удаляемое рабочее пространство сейчас активно,
        // сбрасываем активное (теперь активного не будет).
        if (m_activeWorkspace == id) {
            m_activeWorkspace.reset();
        }

        // Удаляем элемент из вектора.
        m_workspaces.erase(it);
        return true;
    }

    // ── Активация рабочего пространства ──────────────────────

    // Сделать указанное рабочее пространство активным.
    // Если пространство с таким id не найдено, выбрасывает исключение.
    void activate(const std::string& id) {
        // Проверяем, существует ли такое рабочее пространство.
        auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(),
            [&](const Workspace& ws) { return ws.id == id; });

        if (it == m_workspaces.end()) {
            // Если не нашли — сообщаем об ошибке.
            throw std::runtime_error("WorkspaceManager::activate: рабочее пространство с id '" + id + "' не найдено.");
        }

        // Устанавливаем новое активное рабочее пространство.
        m_activeWorkspace = id;
    }

    // Получить идентификатор текущего активного рабочего пространства.
    // Если ни одно не активно, возвращает std::nullopt.
    [[nodiscard]] std::optional<std::string> activeWorkspace() const {
        return m_activeWorkspace;
    }

    // ── Сохранение и загрузка ────────────────────────────────

    // Сохранить текущую конфигурацию рабочих пространств.
    // Пока что просто запоминает текущее состояние в памяти (заглушка).
    // В будущем здесь будет запись в файл.
    bool save() {
        // Здесь пока ничего не делаем, только возвращаем успех.
        // В реальном приложении мы бы сериализовали m_workspaces в JSON/XML/бинарный формат.
        return true;
    }

    // Загрузить конфигурацию рабочих пространств из сохранения.
    // Тоже заглушка — в будущем загрузит из файла.
    bool load() {
        // В будущем восстановит m_workspaces и m_activeWorkspace из файла.
        // Пока что просто возвращаем успех (ничего не меняем).
        return true;
    }

    // ── Доступ к списку рабочих пространств ──────────────────

    // Получить ссылку на вектор всех рабочих пространств (только для чтения).
    [[nodiscard]] const std::vector<Workspace>& workspaces() const {
        return m_workspaces;
    }

private:
    // Все зарегистрированные рабочие пространства.
    std::vector<Workspace> m_workspaces;

    // Идентификатор текущего активного рабочего пространства.
    // std::nullopt означает, что ни одно не активно.
    std::optional<std::string> m_activeWorkspace;
};

} // namespace MirUI