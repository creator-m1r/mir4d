
#pragma once

#include <new>
#include <type_traits>
#include <utility>
#include <stdexcept>

namespace mir {

struct nullopt_t {

    explicit nullopt_t() = default;
};

inline constexpr nullopt_t nullopt{};

template<typename T>
class Optional {
public:

    Optional() noexcept : m_hasValue(false) {}

    Optional(nullopt_t) noexcept : m_hasValue(false) {}

    Optional(const T& value) : m_hasValue(true) {
        new (m_data) T(value);
    }

    Optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_hasValue(true) {
        new (m_data) T(std::move(value));
    }

    Optional(const Optional& other) : m_hasValue(other.m_hasValue) {
        if (m_hasValue) {
            new (m_data) T(other.value());
        }
    }

    Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_hasValue(other.m_hasValue) {
        if (m_hasValue) {
            new (m_data) T(std::move(other.value()));
            other.m_hasValue = false;
        }
    }

    ~Optional() {
        reset();
    }

    Optional& operator=(nullopt_t) noexcept {
        reset();
        return *this;
    }

    Optional& operator=(const T& value) {
        reset();
        new (m_data) T(value);
        m_hasValue = true;
        return *this;
    }

    Optional& operator=(T&& value) noexcept(std::is_nothrow_move_assignable_v<T>) {
        reset();
        new (m_data) T(std::move(value));
        m_hasValue = true;
        return *this;
    }

    Optional& operator=(const Optional& other) {
        if (this != &other) {
            reset();
            if (other.m_hasValue) {
                new (m_data) T(other.value());
                m_hasValue = true;
            }
        }
        return *this;
    }

    Optional& operator=(Optional&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (this != &other) {
            reset();
            if (other.m_hasValue) {
                new (m_data) T(std::move(other.value()));
                m_hasValue = true;
                other.m_hasValue = false;
            }
        }
        return *this;
    }

    [[nodiscard]] bool has_value() const noexcept { return m_hasValue; }

    explicit operator bool() const noexcept { return m_hasValue; }

    T& value() {
        if (!m_hasValue) {
            throw std::runtime_error("Optional::value() — коробочка пуста!");
        }
        return *reinterpret_cast<T*>(m_data);
    }

    const T& value() const {
        if (!m_hasValue) {
            throw std::runtime_error("Optional::value() — коробочка пуста!");
        }
        return *reinterpret_cast<const T*>(m_data);
    }

    T value_or(const T& default_value) const {
        return m_hasValue ? value() : default_value;
    }

    void reset() noexcept {
        if (m_hasValue) {
            reinterpret_cast<T*>(m_data)->~T();
            m_hasValue = false;
        }
    }

    bool operator==(const Optional& other) const {
        if (!m_hasValue && !other.m_hasValue) return true;
        if ( m_hasValue &&  other.m_hasValue) return value() == other.value();
        return false;
    }

    bool operator!=(const Optional& other) const {
        return !(*this == other);
    }

private:

    alignas(T) unsigned char m_data[sizeof(T)];

    bool m_hasValue;
};

template<typename T>
Optional<T> make_optional(T&& value) {
    return Optional<T>(std::forward<T>(value));
}

}