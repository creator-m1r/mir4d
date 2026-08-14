// MirUI/Core/Widget/WidgetTreeSnapshot.hpp
// 🌳 Снимок всего дерева виджетов — полное описание интерфейса в одной структуре.
//
// WidgetTreeSnapshot содержит корневой WidgetSnapshot и общее количество
// виджетов. Этого достаточно, чтобы любой рендерер (SwiftUI, WinUI, WebUI)
// мог построить всё дерево нативных представлений, не обращаясь к C++.
//
// Снимок создаётся UIRuntime или UIContext и может быть передан через
// ABI-границу, сериализован в JSON или использован для сравнения (diff).
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "WidgetSnapshot.hpp"
#include <cstddef>

namespace MirUI {

struct WidgetTreeSnapshot {
    WidgetSnapshot root;           // корневой виджет (обычно Window)
    std::size_t     totalNodes = 0; // общее количество виджетов в дереве

    // ── Операторы сравнения (явная реализация) ──────────────
    // Используем явное сравнение, потому что WidgetSnapshot содержит
    // std::unordered_map, для которого компилятор не может сгенерировать
    // оператор "= default".
    bool operator==(const WidgetTreeSnapshot& other) const {
        return root == other.root && totalNodes == other.totalNodes;
    }

    bool operator!=(const WidgetTreeSnapshot& other) const {
        return !(*this == other);
    }
};

} // namespace MirUI