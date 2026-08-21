// MirUI/Workspace/PanelManager/PanelDescriptor.hpp
// Описание одной панели в рабочем пространстве: её имя, иконка,
// видимость, возможность закрытия и минимальные/предпочтительные размеры.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include <string>
#include "../../Core/Layout/Size.hpp"
#include "../../Foundation/Icons/IconID.hpp"

namespace MirUI {

struct PanelDescriptor {
    // Уникальный идентификатор панели (например, "navigator", "inspector").
    std::string id;

    // Заголовок, который будет отображаться на вкладке или в заголовке панели.
    std::string title;

    // Иконка панели (платформонезависимый строковый идентификатор).
    IconID icon;

    // Видна ли панель в данный момент?
    bool visible = true;

    // Можно ли закрыть панель (если false, кнопка закрытия не показывается).
    bool closable = true;

    // Можно ли панель сделать плавающей (открепить от основного окна)?
    bool floatable = true;

    // Минимальные допустимые размеры панели (меньше она не сожмётся).
    Size minimumSize = { 200.0, 150.0 };

    // Предпочтительные размеры панели (которые она хочет занять при первой загрузке).
    Size preferredSize = { 300.0, 400.0 };
};

} // namespace MirUI