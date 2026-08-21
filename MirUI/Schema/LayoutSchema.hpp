
#pragma once

#include <string>
#include <vector>
#include <algorithm>

namespace MirUI {

struct LayoutDescriptor {
    std::string id;
    std::string name;
    std::string description;
    std::string icon;
    double defaultSpacing = 4.0;
    double defaultPadding = 8.0;
};

class LayoutSchema {
public:

    [[nodiscard]] static const std::vector<LayoutDescriptor>& allLayouts() {
        return layouts();
    }

    [[nodiscard]] static const LayoutDescriptor* find(const std::string& id) {
        auto& lst = layouts();
        auto it = std::find_if(lst.begin(), lst.end(),
            [&id](const LayoutDescriptor& desc) { return desc.id == id; });
        return (it != lst.end()) ? &(*it) : nullptr;
    }

    [[nodiscard]] static std::vector<std::string> allIds() {
        std::vector<std::string> ids;
        for (const auto& desc : layouts()) {
            ids.push_back(desc.id);
        }
        return ids;
    }

private:

    static std::vector<LayoutDescriptor>& layouts() {
        static std::vector<LayoutDescriptor> s_layouts = {
            {
                "vertical",
                "Вертикальная",
                "Дочерние виджеты располагаются друг под другом (сверху вниз)",
                "layout_vertical",
                4.0,
                8.0
            },
            {
                "horizontal",
                "Горизонтальная",
                "Дочерние виджеты располагаются рядом (слева направо)",
                "layout_horizontal",
                4.0,
                8.0
            },
            {
                "grid",
                "Сетка",
                "Виджеты выравниваются по ячейкам таблицы с заданным числом столбцов",
                "layout_grid",
                4.0,
                8.0
            },
            {
                "stack",
                "Стек",
                "Виджеты накладываются друг на друга (как страницы книги)",
                "layout_stack",
                0.0,
                0.0
            },
            {
                "dock",
                "Стыковка (Dock)",
                "Виджеты прикрепляются к краям контейнера: слева, справа, сверху, снизу или заполняют центр",
                "layout_dock",
                0.0,
                0.0
            },
            {
                "absolute",
                "Абсолютное позиционирование",
                "Виджеты располагаются по фиксированным координатам (x, y) относительно родителя",
                "layout_absolute",
                0.0,
                0.0
            }
        };
        return s_layouts;
    }
};

}