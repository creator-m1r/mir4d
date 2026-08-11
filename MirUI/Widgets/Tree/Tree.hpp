// MirUI/Widgets/Tree/Tree.hpp
// Виджет «Дерево» — отображает иерархический список элементов.
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../../Core/Widget/Widget.hpp"
#include "TreeNode.hpp"
#include <vector>
#include <string>
#include <optional>
#include <algorithm>

namespace MirUI {

class Tree : public Widget {
public:
    // Конструктор создаёт виджет с типом Tree.
    Tree()
        : Widget(WidgetType::Tree)
    {}

    // ── Управление узлами ────────────────────────────────────

    // Добавить корневой узел.
    void addNode(const TreeNode& node) {
        m_nodes.push_back(node);
    }

    // Удалить узел (и всех его потомков) по идентификатору.
    // Возвращает true, если узел был найден и удалён.
    bool removeNode(const std::string& id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const TreeNode& n) { return n.id == id; });
        if (it != m_nodes.end()) {
            m_nodes.erase(it);
            // Если удаляемый узел был выделен, сбрасываем выделение.
            if (m_selectedNode && *m_selectedNode == id) {
                m_selectedNode.reset();
            }
            return true;
        }
        // Поиск в детях (рекурсивный обход).
        for (auto& node : m_nodes) {
            if (removeNodeRecursive(node, id)) {
                return true;
            }
        }
        return false;
    }

    // ── Выделение ────────────────────────────────────────────

    // Выделить узел с заданным id.
    void select(const std::string& id) {
        // Снимаем выделение с предыдущего узла.
        if (m_selectedNode) {
            setSelectedRecursive(m_nodes, *m_selectedNode, false);
        }
        m_selectedNode = id;
        setSelectedRecursive(m_nodes, id, true);
    }

    // Получить id выделенного узла или std::nullopt.
    [[nodiscard]] std::optional<std::string> selectedNode() const {
        return m_selectedNode;
    }

    // ── Разворачивание / сворачивание ────────────────────────

    // Развернуть узел с заданным id.
    void expand(const std::string& id) {
        setExpandedRecursive(m_nodes, id, true);
    }

    // Свернуть узел с заданным id.
    void collapse(const std::string& id) {
        setExpandedRecursive(m_nodes, id, false);
    }

    // ── Доступ к узлам ───────────────────────────────────────

    // Ссылка на вектор корневых узлов.
    [[nodiscard]] const std::vector<TreeNode>& nodes() const {
        return m_nodes;
    }

    // ID узла, над которым в данный момент находится курсор мыши.
    [[nodiscard]] const std::string& hoveredNode() const {
        return m_hoveredNode;
    }
    void setHoveredNode(const std::string& id) {
        m_hoveredNode = id;
    }

private:
    std::vector<TreeNode> m_nodes;            // Корневые узлы дерева.
    std::optional<std::string> m_selectedNode; // ID выделенного узла.
    std::string m_hoveredNode;                // ID узла под мышью.

    // Рекурсивно удалить узел по id внутри родительского узла.
    bool removeNodeRecursive(TreeNode& parent, const std::string& id) {
        auto& children = parent.children;
        auto it = std::find_if(children.begin(), children.end(),
            [&](const TreeNode& n) { return n.id == id; });
        if (it != children.end()) {
            children.erase(it);
            if (m_selectedNode && *m_selectedNode == id) {
                m_selectedNode.reset();
            }
            return true;
        }
        for (auto& child : children) {
            if (removeNodeRecursive(child, id)) {
                return true;
            }
        }
        return false;
    }

    // Установить флаг selected для узла с заданным id (рекурсивный обход).
    void setSelectedRecursive(std::vector<TreeNode>& nodes, const std::string& id, bool selected) {
        for (auto& node : nodes) {
            if (node.id == id) {
                node.selected = selected;
                return;
            }
            setSelectedRecursive(node.children, id, selected);
        }
    }

    // Установить флаг expanded для узла с заданным id.
    void setExpandedRecursive(std::vector<TreeNode>& nodes, const std::string& id, bool expanded) {
        for (auto& node : nodes) {
            if (node.id == id) {
                node.expanded = expanded;
                return;
            }
            setExpandedRecursive(node.children, id, expanded);
        }
    }
};

} // namespace MirUI