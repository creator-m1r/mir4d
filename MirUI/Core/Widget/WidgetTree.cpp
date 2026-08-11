// MirUI/Core/Widget/WidgetTree.cpp
// 🌳 Реализация центрального дерева виджетов.
// Содержит логику управления корнем, регистрации/удаления виджетов в индексе,
// поиска по ID, обхода дерева и вспомогательные методы.
// Код вынесен из заголовка для улучшения структуры проекта и ускорения компиляции.
//
// Чистый C++23, без платформенных зависимостей.

#include "WidgetTree.hpp"
#include <stdexcept>   // для std::runtime_error (если понадобится)

namespace MirUI {

// ── Управление корнем ───────────────────────────────────────

void WidgetTree::setRoot(std::unique_ptr<Widget> root) {
    // Очищаем старый индекс перед заменой корня.
    m_index.clear();

    // Перемещаем переданный unique_ptr, становясь единственным владельцем корня.
    m_root = std::move(root);

    // Если новый корень существует, рекурсивно регистрируем всё поддерево в индексе.
    if (m_root) {
        registerSubtree(m_root.get());
    }
}

Widget* WidgetTree::root() const noexcept {
    return m_root.get();   // возвращаем "голый" указатель (владение остаётся у m_root)
}

// ── Регистрация в индексе ───────────────────────────────────

void WidgetTree::registerWidget(Widget* widget) {
    if (widget) {
        // Добавляем или обновляем запись: WidgetID → указатель на виджет.
        m_index[widget->id()] = widget;
    }
}

void WidgetTree::unregisterWidget(WidgetID id) {
    // Удаляем запись из индекса (если она была).
    // Если записи нет, ничего не происходит.
    m_index.erase(id);
}

// ── Поиск ───────────────────────────────────────────────────

Widget* WidgetTree::find(WidgetID id) const {
    auto it = m_index.find(id);
    return (it != m_index.end()) ? it->second : nullptr;
}

Widget* WidgetTree::findParent(WidgetID id) const {
    Widget* w = find(id);
    return w ? w->parent() : nullptr;
}

const std::vector<Widget*>& WidgetTree::findChildren(WidgetID id) const {
    Widget* w = find(id);
    if (w) {
        return w->children();
    }
    // Если виджет не найден, возвращаем ссылку на статический пустой вектор.
    static const std::vector<Widget*> empty;
    return empty;
}

// ── Обход дерева ────────────────────────────────────────────

void WidgetTree::forEach(const std::function<void(Widget*)>& func) const {
    if (m_root) {
        forEachRecursive(m_root.get(), func);
    }
}

// ── Приватные вспомогательные методы ────────────────────────

void WidgetTree::registerSubtree(Widget* w) {
    if (!w) return;
    registerWidget(w);
    for (Widget* child : w->children()) {
        registerSubtree(child);
    }
}

void WidgetTree::forEachRecursive(Widget* w, const std::function<void(Widget*)>& func) const {
    if (!w) return;
    func(w);                       // сначала вызываем для самого виджета
    for (Widget* child : w->children()) {
        forEachRecursive(child, func); // затем рекурсивно для всех детей
    }
}

} // namespace MirUI