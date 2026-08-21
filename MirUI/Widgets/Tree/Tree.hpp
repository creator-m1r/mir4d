
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

    Tree()
        : Widget(WidgetType::Tree)
    {}

    void addNode(const TreeNode& node) {
        m_nodes.push_back(node);
    }

    bool removeNode(const std::string& id) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
            [&](const TreeNode& n) { return n.id == id; });
        if (it != m_nodes.end()) {
            m_nodes.erase(it);

            if (m_selectedNode && *m_selectedNode == id) {
                m_selectedNode.reset();
            }
            return true;
        }

        for (auto& node : m_nodes) {
            if (removeNodeRecursive(node, id)) {
                return true;
            }
        }
        return false;
    }

    void select(const std::string& id) {

        if (m_selectedNode) {
            setSelectedRecursive(m_nodes, *m_selectedNode, false);
        }
        m_selectedNode = id;
        setSelectedRecursive(m_nodes, id, true);
    }

    [[nodiscard]] std::optional<std::string> selectedNode() const {
        return m_selectedNode;
    }

    void expand(const std::string& id) {
        setExpandedRecursive(m_nodes, id, true);
    }

    void collapse(const std::string& id) {
        setExpandedRecursive(m_nodes, id, false);
    }

    [[nodiscard]] const std::vector<TreeNode>& nodes() const {
        return m_nodes;
    }

    [[nodiscard]] const std::string& hoveredNode() const {
        return m_hoveredNode;
    }
    void setHoveredNode(const std::string& id) {
        m_hoveredNode = id;
    }

private:
    std::vector<TreeNode> m_nodes;
    std::optional<std::string> m_selectedNode;
    std::string m_hoveredNode;

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

    void setSelectedRecursive(std::vector<TreeNode>& nodes, const std::string& id, bool selected) {
        for (auto& node : nodes) {
            if (node.id == id) {
                node.selected = selected;
                return;
            }
            setSelectedRecursive(node.children, id, selected);
        }
    }

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

}