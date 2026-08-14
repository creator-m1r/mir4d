#pragma once

#include "TransformProperties.hpp"
#include "../State/StateValue.hpp"

#include <string>
#include <vector>

namespace MirUI
{

struct TransformPropertyRow
{
    std::string name;
    StateValue value;
};

class TransformAdapter
{
public:
    [[nodiscard]] static std::vector<TransformPropertyRow> makeProperties(
        const TransformProperties& properties);
};

} // namespace MirUI
