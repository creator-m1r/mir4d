#pragma once

#include "SelectionState.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Math/Point.hpp"
#include "MirEngine/Math/Vector/Vector.hpp"
#include "MirEngine/Viewport/Camera.hpp"

#include <cmath>
#include <limits>

namespace mir
{

struct PickResult
{
    mir4d::ObjectId objectId{mir4d::InvalidObjectId};
    Scalar distance{std::numeric_limits<Scalar>::max()};
    // Source B-Rep face of the hit triangle (TriangleMesh3::Triangle::sourceFaceId).
    // Zero means the mesh carries no face provenance.
    std::uint64_t faceId{0};

    [[nodiscard]] bool hit() const noexcept { return mir4d::isValidObjectId(objectId); }
};

/// World-space picking ray. Valid for both perspective and orthographic
/// projections: in orthographic mode the origin lies on the near clip plane
/// and the direction is parallel to the view axis.
struct PickRay
{
    Point3 origin{};
    Vector3 direction{};
};

/// CPU ray picker over canonical document meshes.
/// BRep topology remains authoritative; TriangleMesh3 is the render/pick representation.
class RayPicker
{
public:
    /// Screen-space entry point: builds the world ray through a pixel and
    /// delegates to `pick(scene, ray)`.
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const Camera& camera,
        Scalar screenX,
        Scalar screenY,
        std::uint32_t viewportWidth,
        std::uint32_t viewportHeight) noexcept
    {
        const PickRay ray = buildRay(
            camera, screenX, screenY, viewportWidth, viewportHeight);
        return pick(scene, ray);
    }

    /// Picks against a pre-built world-space ray. Used by spatial input
    /// (hand tracking): the caller supplies the ray origin/direction directly
    /// instead of a screen pixel, so no camera projection is needed.
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const PickRay& worldRay) noexcept
    {
        PickResult result{};
        if (worldRay.direction.isZero())
            return result;

        for (const auto& node : scene.nodes())
        {
            if (!node || !node->model() || !node->model()->hasMesh())
                continue;

            const Transform& transform = node->transform();
            const Matrix4 world = transform.matrix();
            const Matrix4 inverseWorld = world.inverse();

            const Vector4 localOrigin4 = inverseWorld * Vector4(worldRay.origin.x, worldRay.origin.y, worldRay.origin.z, Scalar(1));
            const Vector4 localDirection4 = inverseWorld * Vector4(worldRay.direction.x, worldRay.direction.y, worldRay.direction.z, Scalar(0));
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
                const Scalar distance = Vector3{worldHit.x - worldRay.origin.x,
                                                worldHit.y - worldRay.origin.y,
                                                worldHit.z - worldRay.origin.z}.length();

                if (distance < result.distance)
                    result = {node->id(), distance, triangle.sourceFaceId};
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
