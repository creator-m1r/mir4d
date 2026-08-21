// MirUI/Widgets/Tree/TreeNode.hpp
// Data structure representing a single node in a tree widget.
// Pure C++23, no platform dependencies.

#pragma once

#include <string>
#include <vector>
#include "../../Foundation/Icons/IconID.hpp"

namespace MirUI {

struct TreeNode {
    std::string id;
    std::string title;
    IconID icon;

    bool expanded = false;
    bool selected = false;

    std::vector<TreeNode> children;
};

} // namespace MirUI