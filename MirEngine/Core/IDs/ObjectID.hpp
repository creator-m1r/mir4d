#pragma once

#include <cstdint>
#include <functional>

namespace mir {

struct ObjectID {
    std::uint64_t value{0};

    [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
    constexpr explicit operator bool() const noexcept { return valid(); }

    friend constexpr bool operator==(ObjectID, ObjectID) = default;
};

inline constexpr ObjectID InvalidObjectID{};

}

namespace std {
template <>
struct hash<mir::ObjectID> {
    std::size_t operator()(mir::ObjectID id) const noexcept {
        return std::hash<std::uint64_t>{}(id.value);
    }
};
}
