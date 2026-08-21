
#pragma once

#include <new>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <cstddef>

namespace mir {

namespace detail {

    template<typename T, typename... Ts>
    struct IndexOf;

    template<typename T, typename Head, typename... Tail>
    struct IndexOf<T, Head, Tail...> {
        static constexpr size_t value = std::is_same_v<T, Head>
            ? 0
            : 1 + IndexOf<T, Tail...>::value;
    };

    template<typename T>
    struct IndexOf<T> {
        static constexpr size_t value = 0;
    };

    template<typename... Ts>
    struct MaxSize;

    template<typename Head, typename... Tail>
    struct MaxSize<Head, Tail...> {
        static constexpr size_t value =
            sizeof(Head) > MaxSize<Tail...>::value
                ? sizeof(Head)
                : MaxSize<Tail...>::value;
    };

    template<>
    struct MaxSize<> {
        static constexpr size_t value = 1;
    };

    template<typename... Ts>
    struct MaxAlign;

    template<typename Head, typename... Tail>
    struct MaxAlign<Head, Tail...> {
        static constexpr size_t value =
            alignof(Head) > MaxAlign<Tail...>::value
                ? alignof(Head)
                : MaxAlign<Tail...>::value;
    };

    template<>
    struct MaxAlign<> {
        static constexpr size_t value = 1;
    };
}

template<typename... Ts>
class Variant {
public:

    Variant() noexcept(std::is_nothrow_default_constructible_v<
                       typename std::tuple_element<0, std::tuple<Ts...>>::type>)
        : m_activeIndex(0) {
        using FirstType = typename std::tuple_element<0, std::tuple<Ts...>>::type;
        new (m_data) FirstType{};
    }

    template<typename T, typename = std::enable_if_t<
                 (std::is_same_v<T, Ts> || ...)> >
    Variant(const T& value)
        : m_activeIndex(detail::IndexOf<T, Ts...>::value) {
        new (m_data) T(value);
    }

    template<typename T, typename = std::enable_if_t<
                 (std::is_same_v<T, Ts> || ...)> >
    Variant(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_activeIndex(detail::IndexOf<T, Ts...>::value) {
        new (m_data) T(std::move(value));
    }

    Variant(const Variant& other)
        : m_activeIndex(other.m_activeIndex) {
        other.visit([this](const auto& val) {
            using ValueType = std::decay_t<decltype(val)>;
            new (m_data) ValueType(val);
        });
    }

    Variant(Variant&& other) noexcept
        : m_activeIndex(other.m_activeIndex) {
        other.visit([this](auto&& val) {
            using ValueType = std::decay_t<decltype(val)>;
            new (m_data) ValueType(std::move(val));
        });

        other.m_activeIndex = 0;
        new (other.m_data) typename std::tuple_element<0, std::tuple<Ts...>>::type{};
    }

    ~Variant() {
        destroyCurrent();
    }

    Variant& operator=(const Variant& other) {
        if (this != &other) {
            destroyCurrent();
            other.visit([this](const auto& val) {
                using ValueType = std::decay_t<decltype(val)>;
                new (m_data) ValueType(val);
            });
            m_activeIndex = other.m_activeIndex;
        }
        return *this;
    }

    Variant& operator=(Variant&& other) noexcept {
        if (this != &other) {
            destroyCurrent();
            other.visit([this](auto&& val) {
                using ValueType = std::decay_t<decltype(val)>;
                new (m_data) ValueType(std::move(val));
            });
            m_activeIndex = other.m_activeIndex;
        }
        return *this;
    }

    template<typename T, typename... Args>
    void emplace(Args&&... args) {
        static_assert((std::is_same_v<T, Ts> || ...),
                      "Variant::emplace — тип T не входит в список допустимых типов");
        destroyCurrent();
        new (m_data) T(std::forward<Args>(args)...);
        m_activeIndex = detail::IndexOf<T, Ts...>::value;
    }

    [[nodiscard]] size_t index() const noexcept { return m_activeIndex; }

    template<typename T>
    [[nodiscard]] bool is() const noexcept {
        return m_activeIndex == detail::IndexOf<T, Ts...>::value;
    }

    template<typename T>
    T& get() {
        if (!is<T>()) {
            throw std::runtime_error(
                "Variant::get<T>() — неверный тип! "
                "Проверьте тип через is<T>() перед вызовом.");
        }
        return *reinterpret_cast<T*>(m_data);
    }

    template<typename T>
    const T& get() const {
        if (!is<T>()) {
            throw std::runtime_error(
                "Variant::get<T>() — неверный тип! "
                "Проверьте тип через is<T>() перед вызовом.");
        }
        return *reinterpret_cast<const T*>(m_data);
    }

    template<typename Visitor>
    decltype(auto) visit(Visitor&& visitor) {
        return visitImpl(std::forward<Visitor>(visitor),
                         std::index_sequence_for<Ts...>{});
    }

    template<typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const {
        return visitImpl(std::forward<Visitor>(visitor),
                         std::index_sequence_for<Ts...>{});
    }

    bool operator==(const Variant& other) const {
        if (m_activeIndex != other.m_activeIndex) return false;
        return other.visit([this](const auto& val) -> bool {
            using T = std::decay_t<decltype(val)>;
            return get<T>() == val;
        });
    }

    bool operator!=(const Variant& other) const {
        return !(*this == other);
    }

private:

    alignas(detail::MaxAlign<Ts...>::value)
    unsigned char m_data[detail::MaxSize<Ts...>::value];

    size_t m_activeIndex;

    void destroyCurrent() noexcept {
        visit([](auto& val) {
            using T = std::decay_t<decltype(val)>;
            val.~T();
        });
    }

    template<typename Visitor, size_t... Indices>
    decltype(auto) visitImpl(Visitor&& visitor, std::index_sequence<Indices...>) {
        using ReturnType = std::common_type_t<
            decltype(std::forward<Visitor>(visitor)(
                std::declval<Ts&>()))...>;
        ReturnType result;
        ((Indices == m_activeIndex
              ? (result = std::forward<Visitor>(visitor)(
                     *reinterpret_cast<Ts*>(m_data)), true)
              : false) || ...);
        return result;
    }

    template<typename Visitor, size_t... Indices>
    decltype(auto) visitImpl(Visitor&& visitor, std::index_sequence<Indices...>) const {
        using ReturnType = std::common_type_t<
            decltype(std::forward<Visitor>(visitor)(
                std::declval<const Ts&>()))...>;
        ReturnType result;
        ((Indices == m_activeIndex
              ? (result = std::forward<Visitor>(visitor)(
                     *reinterpret_cast<const Ts*>(m_data)), true)
              : false) || ...);
        return result;
    }
};

template<typename T, typename... Ts>
Variant<Ts...> make_variant(T&& value) {
    return Variant<Ts...>(std::forward<T>(value));
}

}