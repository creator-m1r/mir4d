// MirUI/Core/Widget/WidgetTree.hpp
// 🌳 Центральное дерево виджетов — хранит корень и индекс всех виджетов.
// Обеспечивает быстрый поиск виджета по ID, добавление/удаление,
// а также автоматическую регистрацию/удаление из индекса.
// Используется всеми компонентами: LayoutEngine, Renderer, Designer, Inspector.
//
// ⚠️ Важное правило: при добавлении виджета в дерево (через setRoot или
// через addChild родителя) нужно обязательно вызывать registerWidget().
// При удалении — unregisterWidget(). Это гарантирует, что индекс всегда актуален.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "Widget.hpp"
#include <unordered_map>
#include <memory>
#include <functional>

namespace MirUI {

class WidgetTree {
public:
    WidgetTree() = default;
    ~WidgetTree() = default;

    // ── Корень дерева ────────────────────────────────────────
    void setRoot(std::unique_ptr<Widget> root) {
        // Полностью очищаем старый индекс.
        m_index.clear();

        // Устанавливаем нового корня и регистрируем всё поддерево.
        m_root = std::move(root);
        if (m_root) {
            registerSubtree(m_root.get());
        }
    }

    [[nodiscard]] Widget* root() const noexcept {
        return m_root.get();
    }

    // ── Регистрация в индексе ────────────────────────────────
    // Вызывается при любом добавлении виджета в иерархию.
    // Если виджет уже зарегистрирован (дубликат), это безопасно — просто перезапишет.
    void registerWidget(Widget* widget) {
        if (widget) {
            m_index[widget->id()] = widget;
        }
    }

    // Удаляет виджет из индекса. Вызывается при удалении из иерархии.
    void unregisterWidget(WidgetID id) {
        m_index.erase(id);
    }

    // ── Поиск ─────────────────────────────────────────────────
    // Быстрый поиск виджета по ID. Если не найден — nullptr.
    [[nodiscard]] Widget* find(WidgetID id) const {
        auto it = m_index.find(id);
        return (it != m_index.end()) ? it->second : nullptr;
    }

    // Найти родителя виджета (обёртка над Widget::parent()).
    [[nodiscard]] Widget* findParent(WidgetID id) const {
        Widget* w = find(id);
        return w ? w->parent() : nullptr;
    }

    // Найти детей виджета (обёртка над Widget::children()).
    [[nodiscard]] const std::vector<Widget*>& findChildren(WidgetID id) const {
        Widget* w = find(id);
        if (w) {
            return w->children();
        }
        static const std::vector<Widget*> empty;
        return empty;
    }

    // ── Обход дерева ─────────────────────────────────────────
    // Рекурсивный обход в глубину с вызовом callback для каждого виджета.
    void forEach(const std::function<void(Widget*)>& func) const {
        if (m_root) {
            forEachRecursive(m_root.get(), func);
        }
    }

private:
    std::unique_ptr<Widget> m_root;
    std::unordered_map<WidgetID, Widget*> m_index;

    // Рекурсивно регистрирует всё поддерево.
    void registerSubtree(Widget* w) {
        if (!w) return;
        registerWidget(w);
        for (Widget* child : w->children()) {
            registerSubtree(child);
        }
    }

    // Рекурсивный обход.
    void forEachRecursive(Widget* w, const std::function<void(Widget*)>& func) const {
        if (!w) return;
        func(w);
        for (Widget* child : w->children()) {
            forEachRecursive(child, func);
        }
    }
};

} // namespace MirUI