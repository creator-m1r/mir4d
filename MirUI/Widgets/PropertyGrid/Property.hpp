
#pragma once

#include <string>
#include <variant>
#include <cstdint>
#include "../../Core/State/StateValue.hpp"

namespace MirUI {

struct Property {
    std::string id;
    std::string name;
    std::string category;

    StateValue value;
    
    bool readOnly = false;
};

}