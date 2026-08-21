#pragma once

#include "SketchGeometry.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace mir
{

struct SketchVariableRange
{
    std::size_t offset{0};
    std::size_t count{0};
};

class SketchVariableBinding
{
public:
    void bind(std::uint32_t geometryId, SketchVariableRange range)
    {
        ranges_[geometryId] = range;
    }

    [[nodiscard]] const SketchVariableRange* find(std::uint32_t geometryId) const noexcept
    {
        const auto it = ranges_.find(geometryId);
        return it == ranges_.end() ? nullptr : &it->second;
    }

    void clear() noexcept
    {
        ranges_.clear();
    }

private:
    std::unordered_map<std::uint32_t, SketchVariableRange> ranges_;
};

}
