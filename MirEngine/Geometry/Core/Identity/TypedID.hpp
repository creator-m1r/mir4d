#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>

namespace mir
{

template <typename Tag>
class TypedID
{
public:
    using ValueType = std::uint64_t;

    constexpr TypedID() noexcept = default;
    explicit constexpr TypedID(ValueType value) noexcept : value_(value) {}

    [[nodiscard]] constexpr ValueType value() const noexcept { return value_; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return value_ != 0; }

    friend constexpr bool operator==(TypedID lhs, TypedID rhs) noexcept
    {
        return lhs.value_ == rhs.value_;
    }

    friend constexpr bool operator!=(TypedID lhs, TypedID rhs) noexcept
    {
        return lhs.value_ != rhs.value_;
    }

    friend constexpr bool operator<(TypedID lhs, TypedID rhs) noexcept
    {
        return lhs.value_ < rhs.value_;
    }

private:
    ValueType value_{0};
};

struct VertexIDTag;
struct EdgeIDTag;
struct LoopIDTag;
struct FaceIDTag;
struct ShellIDTag;
struct SolidIDTag;
struct BodyIDTag;

using VertexID = TypedID<VertexIDTag>;
using EdgeID = TypedID<EdgeIDTag>;
using LoopID = TypedID<LoopIDTag>;
using FaceID = TypedID<FaceIDTag>;
using ShellID = TypedID<ShellIDTag>;
using SolidID = TypedID<SolidIDTag>;
using BodyID = TypedID<BodyIDTag>;

}

namespace std
{

template <typename Tag>
struct hash<mir::TypedID<Tag>>
{
    std::size_t operator()(const mir::TypedID<Tag>& id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value());
    }
};

}
