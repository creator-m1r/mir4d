#pragma once

#include "TransformProperties.hpp"
#include "TransformAdapter.hpp"

#include <vector>

namespace MirUI
{

class TransformModel
{
public:
    void clear() noexcept
    {
        properties_.clear();
    }

    void update(const TransformProperties& properties)
    {
        properties_ = TransformAdapter::makeProperties(properties);
    }

    [[nodiscard]] const std::vector<TransformPropertyRow>& properties() const noexcept
    {
        return properties_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return properties_.empty();
    }

private:
    std::vector<TransformPropertyRow> properties_{};
};

}
