#pragma once

#include <cstdint>
#include <optional>

namespace mir
{

enum class RenderSelectionType : std::uint8_t
{
    None,
    Vertex,
    Edge,
    Face,
    Solid
};

struct RenderSelection
{
    RenderSelectionType type{RenderSelectionType::None};
    std::uint64_t id{0};

    [[nodiscard]] bool valid() const noexcept
    {
        return type != RenderSelectionType::None && id != 0;
    }

    static RenderSelection none() noexcept { return {}; }
};

} // namespace mir
