
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

    bool create(const Workspace& workspace) {

        auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(),
            [&](const Workspace& ws) { return ws.id == workspace.id; });
        if (it != m_workspaces.end()) {

            return false;
        }

        m_workspaces.push_back(workspace);

        if (m_workspaces.size() == 1) {
            m_activeWorkspace = workspace.id;
        }

        return true;
    }

    bool remove(const std::string& id) {

        auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(),
            [&](const Workspace& ws) { return ws.id == id; });

        if (it == m_workspaces.end()) {
            return false;
        }

        if (m_activeWorkspace == id) {
            m_activeWorkspace.reset();
        }

        m_workspaces.erase(it);
        return true;
    }

    void activate(const std::string& id) {

        auto it = std::find_if(m_workspaces.begin(), m_workspaces.end(),
            [&](const Workspace& ws) { return ws.id == id; });

        if (it == m_workspaces.end()) {

            throw std::runtime_error("WorkspaceManager::activate: рабочее пространство с id '" + id + "' не найдено.");
        }

        m_activeWorkspace = id;
    }

    [[nodiscard]] std::optional<std::string> activeWorkspace() const {
        return m_activeWorkspace;
    }

    bool save() {

        return true;
    }

    bool load() {

        return true;
    }

    [[nodiscard]] const std::vector<Workspace>& workspaces() const {
        return m_workspaces;
    }

private:

    std::vector<Workspace> m_workspaces;

    std::optional<std::string> m_activeWorkspace;
};

}