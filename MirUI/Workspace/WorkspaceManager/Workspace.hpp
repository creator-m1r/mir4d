
#pragma once

#include <string>
#include <vector>

namespace MirUI {

struct Workspace {

    std::string id;

    std::string name;

    std::vector<std::string> panels;
};

}