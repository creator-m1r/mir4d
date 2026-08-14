#pragma once

#include <cstdint>

namespace mir4d
{

using ObjectId = std::uint64_t;

inline constexpr ObjectId InvalidObjectId = 0;

[[nodiscard]] constexpr bool isValidObjectId(ObjectId id) noexcept
{
    return id != InvalidObjectId;
}

} // namespace mir4d
