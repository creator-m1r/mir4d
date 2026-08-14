// MirUI/Workspace/WorkspaceManager/Workspace.hpp
// Структура «Рабочее пространство» (Workspace) — описывает набор панелей,
// которые должны быть видны в определённом режиме работы (Modeling, Assembly и т.д.).
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include <vector>

namespace MirUI {

struct Workspace {
    // Уникальный идентификатор рабочего пространства (например, "modeling", "drafting").
    std::string id;

    // Человекочитаемое название (например, "Моделирование", "Чертёж").
    std::string name;

    // Список идентификаторов панелей, которые должны быть показаны в этом рабочем пространстве.
    // Каждый id должен соответствовать PanelDescriptor::id, зарегистрированному в PanelManager.
    std::vector<std::string> panels;
};

} // namespace MirUI