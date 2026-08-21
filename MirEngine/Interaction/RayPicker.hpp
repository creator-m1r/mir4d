#pragma once

#include "MirEngine/Interaction/PickTypes.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/Vector/Vector.hpp"
#include "MirEngine/Viewport/Camera.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <utility>

namespace mir
{

/// World-space picking ray. Valid for both perspective and orthographic
/// projections: in orthographic mode the origin lies on the near clip plane
/// and the direction is parallel to the view axis.
struct PickRay
{
    Point3 origin{};
    Vector3 direction{};
};

/// Returns the closest distance from a world-space ray to a point.
[[nodiscard]] inline Scalar rayPointGap(const PickRay& ray, const Point3& c) noexcept
{
    const Vector3 oc{c.x - ray.origin.x, c.y - ray.origin.y, c.z - ray.origin.z};
    Scalar t = Vector3::dot(oc, ray.direction);
    if (t < Scalar(0))
        t = Scalar(0);
    const Point3 closest{
        ray.origin.x + ray.direction.x * t,
        ray.origin.y + ray.direction.y * t,
        ray.origin.z + ray.direction.z * t};
    return Vector3{c.x - closest.x, c.y - closest.y, c.z - closest.z}.length();
}

/// CPU ray picker over canonical document meshes.
/// BRep topology remains authoritative; TriangleMesh3 is the render/pick representation.
class RayPicker
{
public:
    /// Broad-phase reject for the world-ray (spatial/hand) pick: returns true
    /// when a node's world bounding sphere is further than `tolerance` from the
    /// ray, so its vertices/edges cannot possibly be selected. The sphere radius
    /// is conservatively expanded by the maximum node scale, so this never
    /// produces a false rejection.
    [[nodiscard]] static bool nodeOutsideRaySphere(
        const PickRay& ray,
        const Transform& transform,
        const TriangleMesh3& mesh,
        Scalar tolerance) noexcept
    {
        if (mesh.vertices.empty())
            return true;
        const Point3 localCenter = meshLocalCenter(mesh);
        const Point3 worldCenter = transform.transformPoint(localCenter);
        const Scalar localRadius = meshLocalRadius(mesh);
        const Scalar maxScale = std::max(
            {transform.scale.x, transform.scale.y, transform.scale.z});
        const Scalar worldRadius = localRadius * maxScale;
        return (rayPointGap(ray, worldCenter) - worldRadius) > tolerance;
    }

private:
    [[nodiscard]] static Point3 meshLocalCenter(const TriangleMesh3& mesh) noexcept
    {
        if (mesh.vertices.empty())
            return Point3::origin();
        const Point3 mn = mesh.boundsMin();
        const Point3 mx = mesh.boundsMax();
        return Point3{
            (mn.x + mx.x) * Scalar(0.5),
            (mn.y + mx.y) * Scalar(0.5),
            (mn.z + mx.z) * Scalar(0.5)};
    }

    [[nodiscard]] static Scalar meshLocalRadius(const TriangleMesh3& mesh) noexcept
    {
        if (mesh.vertices.empty())
            return Scalar(0);
        const Point3 c = meshLocalCenter(mesh);
        const Point3 mx = mesh.boundsMax();
        return Vector3{mx.x - c.x, mx.y - c.y, mx.z - c.z}.length();
    }

public:
    /// Screen-space entry point: builds the world ray through a pixel and
    /// delegates to `pick(scene, ray, filter)`.
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const Camera& camera,
        Scalar screenX,
        Scalar screenY,
        std::uint32_t viewportWidth,
        std::uint32_t viewportHeight) noexcept
    {
        return pick(scene, camera, screenX, screenY,
                    viewportWidth, viewportHeight, PickFilter{});
    }

    /// Screen-space hierarchical pick. Resolves vertices/edges in pixel space
    /// (with the filter tolerances) before falling back to the closest face or
    /// the body, according to the active selection mode.
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const Camera& camera,
        Scalar screenX,
        Scalar screenY,
        std::uint32_t viewportWidth,
        std::uint32_t viewportHeight,
        const PickFilter& filter) noexcept
    {
        PickResult result{};
        if (viewportWidth == 0 || viewportHeight == 0)
            return result;

        const PickRay ray = buildRay(
            camera, screenX, screenY, viewportWidth, viewportHeight);

        const FaceHit face = closestFace(scene, ray);
        const auto project = [&](const Point3& wp) -> std::optional<std::pair<Scalar, Scalar>>
        {
            return projectToScreen(camera, wp, viewportWidth, viewportHeight);
        };

        // Vertex pass: snap to the closest mesh vertex within the pixel radius.
        if (filter.vertex)
        {
            std::optional<SubHit> best;
            for (const auto& node : scene.nodes())
            {
                if (!node || !node->model() || !node->model()->hasMesh())
                    continue;
                const Transform& transform = node->transform();
                const auto& mesh = node->model()->mesh();
                for (std::size_t vi = 0; vi < mesh.vertices.size(); ++vi)
                {
                    const Point3 wp = transform.transformPoint(mesh.vertices[vi]);
                    const auto p = project(wp);
                    if (!p)
                        continue;
                    const Scalar d = std::hypot(p->first - screenX, p->second - screenY);
                    if (d <= static_cast<Scalar>(filter.vertexPixelRadius))
                    {
                        if (!best || d < best->screenDist)
                            best = {node->id(), static_cast<std::uint64_t>(vi), wp, d};
                    }
                }
            }
            if (best)
            {
                result.objectId = best->objectId;
                result.kind = PickKind::Vertex;
                result.elementId = best->elementId;
                result.worldPoint = best->world;
                result.faceId = 0;
                result.distance = best->screenDist;
                return result;
            }
        }

        // Edge pass: snap to the closest triangle edge within the pixel radius.
        if (filter.edge)
        {
            std::optional<SubHit> best;
            for (const auto& node : scene.nodes())
            {
                if (!node || !node->model() || !node->model()->hasMesh())
                    continue;
                const Transform& transform = node->transform();
                const auto& mesh = node->model()->mesh();
                for (std::size_t ti = 0; ti < mesh.triangles.size(); ++ti)
                {
                    const auto& tri = mesh.triangles[ti];
                    const std::size_t edges[3][2] = {
                        {tri.a, tri.b}, {tri.b, tri.c}, {tri.c, tri.a}};
                    for (int e = 0; e < 3; ++e)
                    {
                        const Point3 wa = transform.transformPoint(mesh.vertices[edges[e][0]]);
                        const Point3 wb = transform.transformPoint(mesh.vertices[edges[e][1]]);
                        const auto pa = project(wa);
                        const auto pb = project(wb);
                        if (!pa || !pb)
                            continue;
                        const Scalar d = pointSegmentDistance2D(
                            screenX, screenY, pa->first, pa->second, pb->first, pb->second);
                        if (d <= static_cast<Scalar>(filter.edgePixelRadius))
                        {
                            if (!best || d < best->screenDist)
                            {
                                const Point3 mid{(wa.x + wb.x) * Scalar(0.5),
                                                 (wa.y + wb.y) * Scalar(0.5),
                                                 (wa.z + wb.z) * Scalar(0.5)};
                                best = {node->id(), ti * 3u + static_cast<std::uint64_t>(e), mid, d};
                            }
                        }
                    }
                }
            }
            if (best)
            {
                result.objectId = best->objectId;
                result.kind = PickKind::Edge;
                result.elementId = best->elementId;
                result.worldPoint = best->world;
                result.faceId = 0;
                result.distance = best->screenDist;
                return result;
            }
        }

        if (face.hit)
        {
            if (filter.face)
            {
                result.objectId = face.objectId;
                result.kind = PickKind::Face;
                result.elementId = face.faceId;
                result.worldPoint = face.point;
                result.normal = face.normal;
                result.faceId = face.faceId;
                result.distance = face.distance;
                return result;
            }
            if (filter.body)
            {
                result.objectId = face.objectId;
                result.kind = PickKind::Body;
                result.elementId = 0;
                result.worldPoint = face.point;
                result.normal = face.normal;
                result.faceId = 0;
                result.distance = face.distance;
                return result;
            }
        }

        return result;
    }

    /// Picks against a pre-built world-space ray. Used by spatial input
    /// (hand tracking): the caller supplies the ray origin/direction directly
    /// instead of a screen pixel, so no camera projection is needed.
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const PickRay& worldRay) noexcept
    {
        return pick(scene, worldRay, PickFilter{});
    }

    /// World-ray hierarchical pick. Without a camera, vertices/edges are tested
    /// in world space using a tolerance derived from the pixel radii (scaled to
    /// world units), so spatial hand input respects the active selection mode.
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const PickRay& worldRay,
        const PickFilter& filter) noexcept
    {
        PickResult result{};
        if (worldRay.direction.isZero())
            return result;

        const Vector3 dir = worldRay.direction.normalized();
        const PickRay ray{worldRay.origin, dir};
        const FaceHit face = closestFace(scene, ray);

        const Scalar vertexWorldRadius =
            static_cast<Scalar>(filter.vertexPixelRadius) * Scalar(0.01);
        const Scalar edgeWorldRadius =
            static_cast<Scalar>(filter.edgePixelRadius) * Scalar(0.01);
        // Conservative broad-phase tolerance: a node outside this distance
        // from the ray cannot satisfy either the vertex or edge test.
        const Scalar broadTolerance = std::max(vertexWorldRadius, edgeWorldRadius);

        // Vertex pass (3D point-to-ray distance).
        if (filter.vertex)
        {
            std::optional<SubHit> best;
            for (const auto& node : scene.nodes())
            {
                if (!node || !node->model() || !node->model()->hasMesh())
                    continue;
                const Transform& transform = node->transform();
                const auto& mesh = node->model()->mesh();
                if (nodeOutsideRaySphere(ray, transform, mesh, broadTolerance))
                    continue;
                for (std::size_t vi = 0; vi < mesh.vertices.size(); ++vi)
                {
                    const Point3 wp = transform.transformPoint(mesh.vertices[vi]);
                    const Vector3 op = Vector3{wp.x - ray.origin.x,
                                               wp.y - ray.origin.y,
                                               wp.z - ray.origin.z};
                    const Scalar t = Vector3::dot(op, dir);
                    if (t <= Scalar(0))
                        continue;
                    const Point3 closest{ray.origin.x + dir.x * t,
                                         ray.origin.y + dir.y * t,
                                         ray.origin.z + dir.z * t};
                    const Vector3 diff{wp.x - closest.x,
                                       wp.y - closest.y,
                                       wp.z - closest.z};
                    const Scalar d = diff.length();
                    if (d <= vertexWorldRadius)
                    {
                        if (!best || d < best->screenDist)
                            best = {node->id(), static_cast<std::uint64_t>(vi), wp, d};
                    }
                }
            }
            if (best)
            {
                result.objectId = best->objectId;
                result.kind = PickKind::Vertex;
                result.elementId = best->elementId;
                result.worldPoint = best->world;
                result.faceId = 0;
                result.distance = best->screenDist;
                return result;
            }
        }

        // Edge pass (3D point-to-ray distance of the edge midpoint).
        if (filter.edge)
        {
            std::optional<SubHit> best;
            for (const auto& node : scene.nodes())
            {
                if (!node || !node->model() || !node->model()->hasMesh())
                    continue;
                const Transform& transform = node->transform();
                const auto& mesh = node->model()->mesh();
                if (nodeOutsideRaySphere(ray, transform, mesh, broadTolerance))
                    continue;
                for (std::size_t ti = 0; ti < mesh.triangles.size(); ++ti)
                {
                    const auto& tri = mesh.triangles[ti];
                    const std::size_t edges[3][2] = {
                        {tri.a, tri.b}, {tri.b, tri.c}, {tri.c, tri.a}};
                    for (int e = 0; e < 3; ++e)
                    {
                        const Point3 wa = transform.transformPoint(mesh.vertices[edges[e][0]]);
                        const Point3 wb = transform.transformPoint(mesh.vertices[edges[e][1]]);
                        const Point3 mid{(wa.x + wb.x) * Scalar(0.5),
                                        (wa.y + wb.y) * Scalar(0.5),
                                        (wa.z + wb.z) * Scalar(0.5)};
                        const Vector3 op{mid.x - ray.origin.x,
                                         mid.y - ray.origin.y,
                                         mid.z - ray.origin.z};
                        const Scalar t = Vector3::dot(op, dir);
                        if (t <= Scalar(0))
                            continue;
                        const Point3 closest{ray.origin.x + dir.x * t,
                                             ray.origin.y + dir.y * t,
                                             ray.origin.z + dir.z * t};
                        const Vector3 diff{mid.x - closest.x,
                                           mid.y - closest.y,
                                           mid.z - closest.z};
                        const Scalar d = diff.length();
                        if (d <= edgeWorldRadius)
                        {
                            if (!best || d < best->screenDist)
                                best = {node->id(), ti * 3u + static_cast<std::uint64_t>(e), mid, d};
                        }
                    }
                }
            }
            if (best)
            {
                result.objectId = best->objectId;
                result.kind = PickKind::Edge;
                result.elementId = best->elementId;
                result.worldPoint = best->world;
                result.faceId = 0;
                result.distance = best->screenDist;
                return result;
            }
        }

        if (face.hit)
        {
            if (filter.face)
            {
                result.objectId = face.objectId;
                result.kind = PickKind::Face;
                result.elementId = face.faceId;
                result.worldPoint = face.point;
                result.normal = face.normal;
                result.faceId = face.faceId;
                result.distance = face.distance;
                return result;
            }
            if (filter.body)
            {
                result.objectId = face.objectId;
                result.kind = PickKind::Body;
                result.elementId = 0;
                result.worldPoint = face.point;
                result.normal = face.normal;
                result.faceId = 0;
                result.distance = face.distance;
                return result;
            }
        }

        return result;
    }

    /// Builds the world-space picking ray through a screen pixel.
    /// Works for perspective and orthographic projections alike because it
    /// inverts the composed (projection * view) matrix.
    [[nodiscard]] static PickRay buildRay(
        const Camera& camera,
        Scalar screenX,
        Scalar screenY,
        std::uint32_t viewportWidth,
        std::uint32_t viewportHeight) noexcept
    {
        PickRay ray{};
        if (viewportWidth == 0 || viewportHeight == 0)
            return ray;

        const Matrix4 projection = camera.projectionMatrix();
        const Matrix4 view = camera.viewMatrix();
        const Matrix4 inverse = (projection * view).inverse();

        const Scalar ndcX = (Scalar(2) * screenX / Scalar(viewportWidth)) - Scalar(1);
        // screenY arrives in view-local space with the origin at the bottom
        // (AppKit NSView / OpenGL framebuffer convention, y increasing upward),
        // matching the renderer: NDC y = +1 maps to the top of the viewport.
        const Scalar ndcY = (Scalar(2) * screenY / Scalar(viewportHeight)) - Scalar(1);

        const Vector4 nearClip{ndcX, ndcY, Scalar(-1), Scalar(1)};
        const Vector4 farClip{ndcX, ndcY, Scalar(1), Scalar(1)};
        const Vector4 nearWorld4 = inverse * nearClip;
        const Vector4 farWorld4 = inverse * farClip;

        if (std::abs(nearWorld4.w) <= Scalar(1e-12) ||
            std::abs(farWorld4.w) <= Scalar(1e-12))
            return ray;

        ray.origin = Point3{nearWorld4.x / nearWorld4.w,
                            nearWorld4.y / nearWorld4.w,
                            nearWorld4.z / nearWorld4.w};
        const Point3 farPoint{farWorld4.x / farWorld4.w,
                              farWorld4.y / farWorld4.w,
                              farWorld4.z / farWorld4.w};
        ray.direction = Vector3{farPoint.x - ray.origin.x,
                                farPoint.y - ray.origin.y,
                                farPoint.z - ray.origin.z}.normalized();
        return ray;
    }

private:
    struct FaceHit
    {
        mir4d::ObjectId objectId{mir4d::InvalidObjectId};
        Scalar distance{std::numeric_limits<Scalar>::max()};
        std::uint64_t faceId{0};
        Point3 point{};
        Vector3 normal{};
        bool hit{false};
    };

    struct SubHit
    {
        mir4d::ObjectId objectId{mir4d::InvalidObjectId};
        std::uint64_t elementId{0};
        Point3 world{};
        Scalar screenDist{std::numeric_limits<Scalar>::max()};
    };

    /// Closest triangle/face hit for a world-space ray across all scene nodes.
    static FaceHit closestFace(const Scene& scene, const PickRay& worldRay) noexcept
    {
        FaceHit best{};
        if (worldRay.direction.isZero())
            return best;

        for (const auto& node : scene.nodes())
        {
            if (!node || !node->model() || !node->model()->hasMesh())
                continue;

            const Transform& transform = node->transform();
            const Matrix4 world = transform.matrix();
            const Matrix4 inverseWorld = world.inverse();

            const Vector4 localOrigin4 = inverseWorld * Vector4(
                worldRay.origin.x, worldRay.origin.y, worldRay.origin.z, Scalar(1));
            const Vector4 localDirection4 = inverseWorld * Vector4(
                worldRay.direction.x, worldRay.direction.y, worldRay.direction.z, Scalar(0));
            const Point3 localOrigin{localOrigin4.x, localOrigin4.y, localOrigin4.z};
            const Vector3 localDirection{localDirection4.x, localDirection4.y, localDirection4.z};

            const auto& mesh = node->model()->mesh();
            for (const auto& triangle : mesh.triangles)
            {
                const Point3& a = mesh.vertices[triangle.a];
                const Point3& b = mesh.vertices[triangle.b];
                const Point3& c = mesh.vertices[triangle.c];

                Scalar t = 0;
                if (!intersectsTriangle(localOrigin, localDirection, a, b, c, t))
                    continue;

                const Point3 localHit{localOrigin.x + localDirection.x * t,
                                      localOrigin.y + localDirection.y * t,
                                      localOrigin.z + localDirection.z * t};
                const Point3 worldHit = transform.transformPoint(localHit);

                const Point3 wa = transform.transformPoint(a);
                const Point3 wb = transform.transformPoint(b);
                const Point3 wc = transform.transformPoint(c);
                const Vector3 faceNormal = Vector3::cross(
                    Vector3{wb.x - wa.x, wb.y - wa.y, wb.z - wa.z},
                    Vector3{wc.x - wa.x, wc.y - wa.y, wc.z - wa.z}).normalized();

                const Scalar distance = Vector3{
                    worldHit.x - worldRay.origin.x,
                    worldHit.y - worldRay.origin.y,
                    worldHit.z - worldRay.origin.z}.length();

                if (distance < best.distance)
                {
                    best.objectId = node->id();
                    best.distance = distance;
                    best.faceId = triangle.sourceFaceId;
                    best.point = worldHit;
                    best.normal = faceNormal;
                    best.hit = true;
                }
            }
        }

        return best;
    }

    /// Projects a world point to viewport pixel coordinates (origin bottom-left).
    /// Returns nullopt for points behind the camera or outside the clip volume.
    [[nodiscard]] static std::optional<std::pair<Scalar, Scalar>> projectToScreen(
        const Camera& camera,
        const Point3& world,
        std::uint32_t viewportWidth,
        std::uint32_t viewportHeight) noexcept
    {
        const Matrix4 vp = camera.projectionMatrix() * camera.viewMatrix();
        const Vector4 clip = vp * Vector4(world.x, world.y, world.z, Scalar(1));
        if (std::abs(clip.w) <= Scalar(1e-9))
            return std::nullopt;
        const Scalar inv = Scalar(1) / clip.w;
        const Scalar ndcX = clip.x * inv;
        const Scalar ndcY = clip.y * inv;
        if (ndcX < Scalar(-1) || ndcX > Scalar(1) || ndcY < Scalar(-1) || ndcY > Scalar(1))
            return std::nullopt;
        const Scalar px = (ndcX + Scalar(1)) * Scalar(0.5) * static_cast<Scalar>(viewportWidth);
        const Scalar py = (ndcY + Scalar(1)) * Scalar(0.5) * static_cast<Scalar>(viewportHeight);
        return std::make_pair(px, py);
    }

    static Scalar pointSegmentDistance2D(
        Scalar px, Scalar py, Scalar ax, Scalar ay, Scalar bx, Scalar by) noexcept
    {
        const Scalar dx = bx - ax;
        const Scalar dy = by - ay;
        const Scalar len2 = dx * dx + dy * dy;
        Scalar t = len2 > Scalar(0) ? ((px - ax) * dx + (py - ay) * dy) / len2 : Scalar(0);
        if (t < Scalar(0))
            t = Scalar(0);
        if (t > Scalar(1))
            t = Scalar(1);
        const Scalar cx = ax + t * dx;
        const Scalar cy = ay + t * dy;
        return std::hypot(px - cx, py - cy);
    }

    static bool intersectsTriangle(const Point3& origin,
                                   const Vector3& direction,
                                   const Point3& a,
                                   const Point3& b,
                                   const Point3& c,
                                   Scalar& t) noexcept
    {
        constexpr Scalar epsilon = Scalar(1e-12);
        const Vector3 edge1{b.x - a.x, b.y - a.y, b.z - a.z};
        const Vector3 edge2{c.x - a.x, c.y - a.y, c.z - a.z};
        const Vector3 p = Vector3::cross(direction, edge2);
        const Scalar det = Vector3::dot(edge1, p);
        if (std::abs(det) <= epsilon)
            return false;

        const Scalar invDet = Scalar(1) / det;
        const Vector3 s{origin.x - a.x, origin.y - a.y, origin.z - a.z};
        const Scalar u = invDet * Vector3::dot(s, p);
        if (u < Scalar(0) || u > Scalar(1))
            return false;

        const Vector3 q = Vector3::cross(s, edge1);
        const Scalar v = invDet * Vector3::dot(direction, q);
        if (v < Scalar(0) || u + v > Scalar(1))
            return false;

        t = invDet * Vector3::dot(edge2, q);
        return t >= Scalar(0);
    }
};

} // namespace mir
