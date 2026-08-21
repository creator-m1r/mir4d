
#pragma once

#include <cstddef>
#include <functional>

namespace mir {

class Count {
public:

    constexpr Count() noexcept = default;

    explicit constexpr Count(std::size_t value) noexcept
        : m_value(value)
    {}

    [[nodiscard]] constexpr std::size_t value() const noexcept {
        return m_value;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return m_value == 0;
    }

    friend constexpr bool operator==(Count lhs, Count rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(Count lhs, Count rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(Count lhs, Count rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(Count lhs, Count rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(Count lhs, Count rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(Count lhs, Count rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    std::size_t m_value = 0;
};

}

namespace std {
template <>
struct hash<mir::Count> {
    std::size_t operator()(const mir::Count& count) const noexcept {
        return hash<std::size_t>{}(count.value());
    }
};
}