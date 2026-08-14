#pragma once

#include "SketchGeometry.hpp"

#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>

namespace mir
{

class SketchSelection
{
public:
    void select(std::uint32_t geometryId, bool additive = false)
    {
        if (!additive)
            selected_.clear();

        if (geometryId != 0)
            selected_.insert(geometryId);
    }

    void toggle(std::uint32_t geometryId)
    {
        if (geometryId == 0)
            return;

        if (selected_.contains(geometryId))
            selected_.erase(geometryId);
        else
            selected_.insert(geometryId);
    }

    void clear() noexcept
    {
        selected_.clear();
    }

    [[nodiscard]] bool contains(std::uint32_t geometryId) const
    {
        return selected_.contains(geometryId);
    }

    [[nodiscard]] std::vector<std::uint32_t> ids() const
    {
        return {selected_.begin(), selected_.end()};
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return selected_.size();
    }

private:
    std::unordered_set<std::uint32_t> selected_;
};

} // namespace mir
