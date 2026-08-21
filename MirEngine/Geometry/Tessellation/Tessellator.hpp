#pragma once

#include "../Solid/FacetedSolid.hpp"
#include "TriangleMesh.hpp"

#include <stdexcept>

namespace mir
{

/// Converts the explicit triangular boundary representation into a render mesh.
class Tessellator3
{
public:
    [[nodiscard]] static TriangleMesh3 create(const Solid3& solid)
    {
        if (!solid.isValid())
            throw std::invalid_argument("Tessellator3 requires a valid Solid3");

        TriangleMesh3 mesh;
        mesh.vertices = solid.vertices();
        mesh.triangles.reserve(solid.triangles().size());

        for (const Solid3::Triangle& triangle : solid.triangles())
            mesh.triangles.push_back({triangle.a, triangle.b, triangle.c});

        mesh.normals.assign(mesh.vertices.size(), Vector3::zero());
        for (const TriangleMesh3::Triangle& triangle : mesh.triangles)
        {
            const Point3& a = mesh.vertices[triangle.a];
            const Point3& b = mesh.vertices[triangle.b];
            const Point3& c = mesh.vertices[triangle.c];
            const Vector3 normal = (b - a).cross(c - a);
            if (normal.lengthSquared() <= Scalar(1e-24)) continue;
            mesh.normals[triangle.a] += normal;
            mesh.normals[triangle.b] += normal;
            mesh.normals[triangle.c] += normal;
        }

        for (Vector3& normal : mesh.normals) normal.normalize();
        if (!mesh.isValid()) throw std::runtime_error("Tessellator3 produced an invalid TriangleMesh3");
        return mesh;
    }
};

} // namespace mir
