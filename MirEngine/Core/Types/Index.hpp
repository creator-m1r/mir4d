
#pragma once

#include <cstddef>
#include <functional>

namespace mir {

class Index {
public:

    constexpr Index() noexcept = default;

    explicit constexpr Index(std::size_t value) noexcept
        : m_value(value)
    {}

    [[nodiscard]] constexpr std::size_t value() const noexcept {
        return m_value;
    }

    friend constexpr bool operator==(Index lhs, Index rhs) noexcept {
        return lhs.m_value == rhs.m_value;
    }
    friend constexpr bool operator!=(Index lhs, Index rhs) noexcept {
        return lhs.m_value != rhs.m_value;
    }
    friend constexpr bool operator<(Index lhs, Index rhs) noexcept {
        return lhs.m_value < rhs.m_value;
    }
    friend constexpr bool operator<=(Index lhs, Index rhs) noexcept {
        return lhs.m_value <= rhs.m_value;
    }
    friend constexpr bool operator>(Index lhs, Index rhs) noexcept {
        return lhs.m_value > rhs.m_value;
    }
    friend constexpr bool operator>=(Index lhs, Index rhs) noexcept {
        return lhs.m_value >= rhs.m_value;
    }

private:
    std::size_t m_value = 0;
};

}

namespace std {
template <>
struct hash<mir::Index> {
    std::size_t operator()(const mir::Index& idx) const noexcept {
        return hash<std::size_t>{}(idx.value());
    }
};
}