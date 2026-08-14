#pragma once

// MirEngine/BRep/Core/BRepHandles.hpp
// Type-safe non-owning handles for topology and geometry records.

#include "MirEngine/BRep/Topology/BRepTypes.hpp"

#include <functional>

namespace mir
{

template <BRepShapeType TypeTag>
struct BRepTopoHandle
{
    BRepIndex index{InvalidBRepIndex};

    [[nodiscard]] constexpr bool valid() const noexcept
    {
        return isValidBRepIndex(index);
    }

    [[nodiscard]] static constexpr BRepShapeType type() noexcept
    {
        return TypeTag;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BRepTopoHandle a,
        BRepTopoHandle b) noexcept
    {
        return a.index == b.index;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepTopoHandle a,
        BRepTopoHandle b) noexcept
    {
        return !(a == b);
    }
};

using BRepVertexHandle = BRepTopoHandle<BRepShapeType::Vertex>;
using BRepEdgeHandle   = BRepTopoHandle<BRepShapeType::Edge>;
using BRepWireHandle   = BRepTopoHandle<BRepShapeType::Wire>;
using BRepFaceHandle   = BRepTopoHandle<BRepShapeType::Face>;
using BRepShellHandle  = BRepTopoHandle<BRepShapeType::Shell>;
using BRepSolidHandle  = BRepTopoHandle<BRepShapeType::Solid>;

struct BRepCurveHandle
{
    BRepIndex index{InvalidBRepIndex};

    [[nodiscard]] constexpr bool valid() const noexcept
    {
        return isValidBRepIndex(index);
    }

    [[nodiscard]] friend constexpr bool operator==(
        BRepCurveHandle a,
        BRepCurveHandle b) noexcept
    {
        return a.index == b.index;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepCurveHandle a,
        BRepCurveHandle b) noexcept
    {
        return !(a == b);
    }
};

struct BRepSurfaceHandle
{
    BRepIndex index{InvalidBRepIndex};

    [[nodiscard]] constexpr bool valid() const noexcept
    {
        return isValidBRepIndex(index);
    }

    [[nodiscard]] friend constexpr bool operator==(
        BRepSurfaceHandle a,
        BRepSurfaceHandle b) noexcept
    {
        return a.index == b.index;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepSurfaceHandle a,
        BRepSurfaceHandle b) noexcept
    {
        return !(a == b);
    }
};

struct BRepPointHandle
{
    BRepIndex index{InvalidBRepIndex};

    [[nodiscard]] constexpr bool valid() const noexcept
    {
        return isValidBRepIndex(index);
    }

    [[nodiscard]] friend constexpr bool operator==(
        BRepPointHandle a,
        BRepPointHandle b) noexcept
    {
        return a.index == b.index;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepPointHandle a,
        BRepPointHandle b) noexcept
    {
        return !(a == b);
    }
};

struct BRepOrientedEdge
{
    BRepEdgeHandle edge{};
    BRepOrientation orientation{BRepOrientation::Forward};

    [[nodiscard]] constexpr bool valid() const noexcept { return edge.valid(); }

    [[nodiscard]] friend constexpr bool operator==(
        BRepOrientedEdge a,
        BRepOrientedEdge b) noexcept
    {
        return a.edge == b.edge && a.orientation == b.orientation;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepOrientedEdge a,
        BRepOrientedEdge b) noexcept
    {
        return !(a == b);
    }

    [[nodiscard]] constexpr BRepOrientedEdge reversed() const noexcept
    {
        return {edge, mir::reversed(orientation)};
    }
};

struct BRepOrientedWire
{
    BRepWireHandle wire{};
    BRepOrientation orientation{BRepOrientation::Forward};

    [[nodiscard]] constexpr bool valid() const noexcept { return wire.valid(); }

    [[nodiscard]] friend constexpr bool operator==(
        BRepOrientedWire a,
        BRepOrientedWire b) noexcept
    {
        return a.wire == b.wire && a.orientation == b.orientation;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepOrientedWire a,
        BRepOrientedWire b) noexcept
    {
        return !(a == b);
    }
};

struct BRepOrientedFace
{
    BRepFaceHandle face{};
    BRepOrientation orientation{BRepOrientation::Forward};

    [[nodiscard]] constexpr bool valid() const noexcept { return face.valid(); }

    [[nodiscard]] friend constexpr bool operator==(
        BRepOrientedFace a,
        BRepOrientedFace b) noexcept
    {
        return a.face == b.face && a.orientation == b.orientation;
    }

    [[nodiscard]] friend constexpr bool operator!=(
        BRepOrientedFace a,
        BRepOrientedFace b) noexcept
    {
        return !(a == b);
    }
};

} // namespace mir

namespace std
{

template <mir::BRepShapeType TypeTag>
struct hash<mir::BRepTopoHandle<TypeTag>>
{
    size_t operator()(mir::BRepTopoHandle<TypeTag> handle) const noexcept
    {
        return hash<mir::BRepIndex>{}(handle.index);
    }
};

template <>
struct hash<mir::BRepCurveHandle>
{
    size_t operator()(mir::BRepCurveHandle handle) const noexcept
    {
        return hash<mir::BRepIndex>{}(handle.index);
    }
};

template <>
struct hash<mir::BRepSurfaceHandle>
{
    size_t operator()(mir::BRepSurfaceHandle handle) const noexcept
    {
        return hash<mir::BRepIndex>{}(handle.index);
    }
};

template <>
struct hash<mir::BRepPointHandle>
{
    size_t operator()(mir::BRepPointHandle handle) const noexcept
    {
        return hash<mir::BRepIndex>{}(handle.index);
    }
};

} // namespace std
