#pragma once

#include <cstdint>
#include <functional>

namespace mir {

struct ComponentID {
    std::uint64_t value{0};

    [[nodiscard]] constexpr bool valid() const noexcept { return value != 0; }
    constexpr explicit operator bool() const noexcept { return valid(); }

    friend constexpr bool operator==(ComponentID, ComponentID) = default;
};

inline constexpr ComponentID InvalidComponentID{};

}

namespace std {
template <>
struct hash<mir::ComponentID> {
    std::size_t operator()(mir::ComponentID id) const noexcept {
        return std::hash<std::uint64_t>{}(id.value);
    }
};
}
