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

    [[nodiscard]] bool hit() const noexcept { return mir4d::isValidObjectId(objectId); }
};

/// CPU ray picker over canonical document meshes.
/// BRep topology remains authoritative; TriangleMesh3 is the render/pick representation.
class RayPicker
{
public:
    [[nodiscard]] static PickResult pick(
        const Scene& scene,
        const Camera& camera,
        Scalar screenX,
        Scalar screenY,
        std::uint32_t viewportWidth,
        std::uint32_t viewportHeight) noexcept
    {
        PickResult result{};
        if (viewportWidth == 0 || viewportHeight == 0)
            return result;

        const Matrix4 projection = camera.projectionMatrix();
        const Matrix4 view = camera.viewMatrix();
        const Matrix4 inverse = (projection * view).inverse();

        const Scalar ndcX = (Scalar(2) * screenX / Scalar(viewportWidth)) - Scalar(1);
        const Scalar ndcY = Scalar(1) - (Scalar(2) * screenY / Scalar(viewportHeight));

        const Vector4 nearClip{ndcX, ndcY, Scalar(-1), Scalar(1)};
        const Vector4 farClip{ndcX, ndcY, Scalar(1), Scalar(1)};
        const Vector4 nearWorld4 = inverse * nearClip;
        const Vector4 farWorld4 = inverse * farClip;

        if (std::abs(nearWorld4.w) <= Scalar(1e-12) ||
            std::abs(farWorld4.w) <= Scalar(1e-12))
            return result;

        const Point3 origin{nearWorld4.x / nearWorld4.w,
                            nearWorld4.y / nearWorld4.w,
                            nearWorld4.z / nearWorld4.w};
        const Point3 farPoint{farWorld4.x / farWorld4.w,
                              farWorld4.y / farWorld4.w,
                              farWorld4.z / farWorld4.w};
        const Vector3 direction = Vector3{farPoint.x - origin.x,
                                          farPoint.y - origin.y,
                                          farPoint.z - origin.z}.normalized();

        for (const auto& node : scene.nodes())
        {
            if (!node || !node->model() || !node->model()->hasMesh())
                continue;

            const Transform& transform = node->transform();
            const Matrix4 world = transform.matrix();
            const Matrix4 inverseWorld = world.inverse();

            const Vector4 localOrigin4 = inverseWorld * Vector4(origin.x, origin.y, origin.z, Scalar(1));
            const Vector4 localDirection4 = inverseWorld * Vector4(direction.x, direction.y, direction.z, Scalar(0));
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
                const Scalar distance = Vector3{worldHit.x - origin.x,
                                                worldHit.y - origin.y,
                                                worldHit.z - origin.z}.length();

                if (distance < result.distance)
                    result = {node->id(), distance};
            }
        }

        return result;
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
