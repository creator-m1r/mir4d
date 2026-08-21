
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

}