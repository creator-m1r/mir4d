#pragma once

#include "MirEngine/Core/Identity/ObjectId.hpp"
#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/Vector/Vector.hpp"

#include <cstdint>
#include <limits>

namespace mir
{

/// Hierarchical pick classification used by both mouse (screen ray) and
/// spatial (hand world ray) input. Priority for sub-object selection is
/// Vertex > Edge > Face > Body, but the active selection mode filters which
/// kinds are allowed (see PickFilter).
enum class PickKind
{
    None,
    Vertex,
    Edge,
    Face,
    Body
};

/// Result of a pick query. `kind == None` means nothing was hit.
/// For an object-level hit `kind == Body` and `elementId == 0`.
/// For a sub-object hit `elementId` carries the mesh-local index
/// (vertex index, edge id, or B-Rep/source face id).
struct PickResult
{
    mir4d::ObjectId objectId{mir4d::InvalidObjectId};
    Scalar distance{std::numeric_limits<Scalar>::max()};

    /// Legacy source B-Rep face id of the hit triangle (0 = whole object / no
    /// face provenance). Mirrors `elementId` when `kind == Face`.
    std::uint64_t faceId{0};

    PickKind kind{PickKind::None};
    std::uint64_t elementId{0};

    /// World-space hit position (valid when `hit()`).
    Point3 worldPoint{};
    /// Surface normal at the hit (valid for face hits; zero otherwise).
    Vector3 normal{};

    [[nodiscard]] bool hit() const noexcept { return mir4d::isValidObjectId(objectId); }
};

/// Controls which sub-object kinds a pick may return and the screen-space
/// tolerances (in pixels) used to make vertices and edges clickable.
struct PickFilter
{
    bool body{true};
    bool face{true};
    bool edge{true};
    bool vertex{true};

    float vertexPixelRadius{10.f};
    float edgePixelRadius{8.f};
};

/// Builds a pick filter from a coarse selection mode.
/// 0 = Body, 1 = Face, 2 = Edge, 3 = Vertex.
[[nodiscard]] inline PickFilter makePickFilter(int mode) noexcept
{
    PickFilter f{};
    switch (mode)
    {
        case 0: // Body
            f.body = true;
            f.face = f.edge = f.vertex = false;
            break;
        case 1: // Face
            f.face = true;
            f.vertex = true; // face mode may still snap to nearby vertices
            f.body = f.edge = false;
            break;
        case 2: // Edge
            f.edge = true;
            f.body = f.face = f.vertex = false;
            break;
        case 3: // Vertex
            f.vertex = true;
            f.body = f.face = f.edge = false;
            break;
        default:
            break;
    }
    return f;
}

} // namespace mir
