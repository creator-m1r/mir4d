#pragma once

// MirEngine/BRep/Tessellator/BRepTessellator.hpp
// Планарная MVP-триангуляция BRep для render mesh.

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Geometry/BRepAdaptor.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Math/Point.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mir
{

struct BRepTessellationOptions
{
    double deflection{0.1};
    bool includeHoles{true};
};

class BRepTessellator
{
public:
    [[nodiscard]] static TriangleMesh3 tessellateSolid(
        const BRepModel& model,
        BRepSolidHandle solidHandle,
        BRepTessellationOptions options = {})
    {
        TriangleMesh3 mesh{};
        const BRepSolid* solid = model.topology().solid(solidHandle);
        if (!solid) return mesh;

        for (BRepShellHandle shellHandle : solid->shells)
        {
            const BRepShell* shell = model.topology().shell(shellHandle);
            if (!shell) continue;
            for (const BRepOrientedFace& orientedFace : shell->faces)
                appendFace(model, orientedFace.face, mesh, options);
        }
        return mesh;
    }

    [[nodiscard]] static TriangleMesh3 tessellateModel(
        const BRepModel& model,
        BRepTessellationOptions options = {})
    {
        TriangleMesh3 mesh{};
        for (BRepSolidHandle solidHandle : model.rootSolids())
            appendMesh(mesh, tessellateSolid(model, solidHandle, options));
        return mesh;
    }

private:
    struct Vec2 { double x{0.0}; double y{0.0}; };
    struct FaceFrame { Vector3 origin{}; Vector3 normal{}; Vector3 xDir{}; Vector3 yDir{}; };

    static void appendMesh(TriangleMesh3& dst, const TriangleMesh3& src)
    {
        const std::size_t base = dst.vertices.size();
        dst.vertices.insert(dst.vertices.end(), src.vertices.begin(), src.vertices.end());
        for (const auto& tri : src.triangles)
            dst.triangles.push_back(
                {tri.a + base, tri.b + base, tri.c + base, tri.sourceFaceId});
    }

    static void appendFace(const BRepModel& model,
                           BRepFaceHandle faceHandle,
                           TriangleMesh3& mesh,
                           const BRepTessellationOptions& options)
    {
        const BRepFace* face = model.topology().face(faceHandle);
        if (!face) return;

        BRepAdaptor_Surface surfaceAdaptor(model, faceHandle);
        if (!surfaceAdaptor.isBound() || surfaceAdaptor.type() != BRepSurfaceType::Plane)
            return;

        const BRepSurfaceGeometry* surface = model.geometry().surface(face->surface);
        if (!surface) return;

        FaceFrame frame{};
        frame.origin = surface->plane.location;
        frame.normal = surface->plane.normal.normalized();
        frame.xDir = surface->plane.xDir.normalized();
        frame.yDir = Vector3::cross(frame.normal, frame.xDir).normalized();

        std::vector<Vector3> outer3d = sampleWirePoints(model, face->outer.wire);
        if (outer3d.size() < 3) return;

        std::vector<Vec2> outer2d;
        outer2d.reserve(outer3d.size());
        for (const Vector3& p : outer3d) outer2d.push_back(toLocal(frame, p));

        if (signedArea(outer2d) < 0.0)
        {
            std::reverse(outer2d.begin(), outer2d.end());
            std::reverse(outer3d.begin(), outer3d.end());
        }

        std::vector<std::vector<Vec2>> holes2d;
        std::vector<std::vector<Vector3>> holes3d;
        if (options.includeHoles)
        {
            for (const BRepOrientedWire& inner : face->inners)
            {
                std::vector<Vector3> hole3d = sampleWirePoints(model, inner.wire);
                if (hole3d.size() < 3) continue;
                std::vector<Vec2> hole2d;
                hole2d.reserve(hole3d.size());
                for (const Vector3& p : hole3d) hole2d.push_back(toLocal(frame, p));
                if (signedArea(hole2d) > 0.0)
                {
                    std::reverse(hole2d.begin(), hole2d.end());
                    std::reverse(hole3d.begin(), hole3d.end());
                }
                holes2d.push_back(std::move(hole2d));
                holes3d.push_back(std::move(hole3d));
            }
        }

        std::vector<Vector3> combined3d;
        std::vector<Vec2> combined2d;
        if (holes2d.empty())
        {
            combined2d = outer2d;
            combined3d = outer3d;
        }
        else
        {
            combineOuterAndHoles(outer2d, outer3d, holes2d, holes3d, combined2d, combined3d);
        }

        const auto tris = earClip(combined2d);
        if (tris.empty()) return;

        const std::size_t base = mesh.vertices.size();
        for (const Vector3& p : combined3d)
            mesh.vertices.push_back(Point3{p.x, p.y, p.z});
        const std::uint64_t faceId = static_cast<std::uint64_t>(faceHandle.index);
        for (const auto& tri : tris)
            mesh.triangles.push_back({base + tri[0], base + tri[1], base + tri[2], faceId});
    }

    [[nodiscard]] static std::vector<Vector3> sampleWirePoints(const BRepModel& model, BRepWireHandle wireHandle)
    {
        std::vector<Vector3> points;
        const BRepWire* wire = model.topology().wire(wireHandle);
        if (!wire) return points;
        for (const BRepOrientedEdge& oriented : wire->edges)
        {
            const BRepEdge* edge = model.topology().edge(oriented.edge);
            if (!edge) continue;
            const BRepVertexHandle start = isForward(oriented.orientation) ? edge->start : edge->end;
            points.push_back(pointOf(model, start));
        }
        return points;
    }

    [[nodiscard]] static Vector3 pointOf(const BRepModel& model, BRepVertexHandle vertexHandle) noexcept
    {
        const BRepVertex* vertex = model.topology().vertex(vertexHandle);
        if (!vertex) return Vector3::zero();
        const BRepPointGeometry* point = model.geometry().point(vertex->point);
        return point ? point->point : Vector3::zero();
    }

    [[nodiscard]] static Vec2 toLocal(const FaceFrame& frame, const Vector3& p) noexcept
    {
        const Vector3 d = p - frame.origin;
        return {Vector3::dot(d, frame.xDir), Vector3::dot(d, frame.yDir)};
    }

    [[nodiscard]] static double signedArea(const std::vector<Vec2>& poly) noexcept
    {
        if (poly.size() < 3) return 0.0;
        double area = 0.0;
        for (std::size_t i = 0, n = poly.size(); i < n; ++i)
        {
            const Vec2& a = poly[i];
            const Vec2& b = poly[(i + 1) % n];
            area += a.x * b.y - b.x * a.y;
        }
        return 0.5 * area;
    }

    static void combineOuterAndHoles(const std::vector<Vec2>& outer2d,
                                     const std::vector<Vector3>& outer3d,
                                     const std::vector<std::vector<Vec2>>& holes2d,
                                     const std::vector<std::vector<Vector3>>& holes3d,
                                     std::vector<Vec2>& out2d,
                                     std::vector<Vector3>& out3d)
    {
        out2d = outer2d;
        out3d = outer3d;
        for (std::size_t h = 0; h < holes2d.size(); ++h)
        {
            const auto& hole2d = holes2d[h];
            const auto& hole3d = holes3d[h];
            if (hole2d.size() < 3) continue;

            std::size_t holeIdx = 0;
            double maxX = hole2d[0].x;
            for (std::size_t i = 1; i < hole2d.size(); ++i)
                if (hole2d[i].x > maxX) { maxX = hole2d[i].x; holeIdx = i; }

            std::size_t outerIdx = 0;
            double bestDist = 1.0e300;
            for (std::size_t i = 0; i < out2d.size(); ++i)
            {
                const double dx = out2d[i].x - hole2d[holeIdx].x;
                const double dy = out2d[i].y - hole2d[holeIdx].y;
                const double d = dx * dx + dy * dy;
                if (d < bestDist) { bestDist = d; outerIdx = i; }
            }

            std::vector<Vec2> next2d;
            std::vector<Vector3> next3d;
            next2d.reserve(out2d.size() + hole2d.size() + 2);
            next3d.reserve(next2d.capacity());

            for (std::size_t i = 0; i <= outerIdx; ++i) { next2d.push_back(out2d[i]); next3d.push_back(out3d[i]); }
            for (std::size_t k = 0; k < hole2d.size(); ++k)
            {
                const std::size_t idx = (holeIdx + k) % hole2d.size();
                next2d.push_back(hole2d[idx]); next3d.push_back(hole3d[idx]);
            }
            next2d.push_back(hole2d[holeIdx]); next3d.push_back(hole3d[holeIdx]);
            next2d.push_back(out2d[outerIdx]); next3d.push_back(out3d[outerIdx]);
            for (std::size_t i = outerIdx + 1; i < out2d.size(); ++i) { next2d.push_back(out2d[i]); next3d.push_back(out3d[i]); }
            out2d.swap(next2d);
            out3d.swap(next3d);
        }
    }

    [[nodiscard]] static bool pointInTriangle(const Vec2& p, const Vec2& a, const Vec2& b, const Vec2& c) noexcept
    {
        const auto sign = [](const Vec2& p1, const Vec2& p2, const Vec2& p3) noexcept
        { return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y); };
        const double d1 = sign(p, a, b), d2 = sign(p, b, c), d3 = sign(p, c, a);
        const bool hasNeg = d1 < 0.0 || d2 < 0.0 || d3 < 0.0;
        const bool hasPos = d1 > 0.0 || d2 > 0.0 || d3 > 0.0;
        return !(hasNeg && hasPos);
    }

    [[nodiscard]] static std::vector<std::array<std::size_t, 3>> earClip(const std::vector<Vec2>& polygon)
    {
        std::vector<std::array<std::size_t, 3>> result;
        const std::size_t n0 = polygon.size();
        if (n0 < 3) return result;

        std::vector<std::size_t> idx(n0);
        for (std::size_t i = 0; i < n0; ++i) idx[i] = i;

        auto isEar = [&](std::size_t i0, std::size_t i1, std::size_t i2) noexcept
        {
            const Vec2& a = polygon[idx[i0]], &b = polygon[idx[i1]], &c = polygon[idx[i2]];
            const double cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross <= 0.0) return false;
            for (std::size_t k = 0; k < idx.size(); ++k)
            {
                if (k == i0 || k == i1 || k == i2) continue;
                if (pointInTriangle(polygon[idx[k]], a, b, c)) return false;
            }
            return true;
        };

        std::size_t guard = 0;
        while (idx.size() > 3 && guard < n0 * n0)
        {
            ++guard;
            bool clipped = false;
            for (std::size_t i = 0; i < idx.size(); ++i)
            {
                const std::size_t i0 = (i + idx.size() - 1) % idx.size();
                const std::size_t i2 = (i + 1) % idx.size();
                if (!isEar(i0, i, i2)) continue;
                result.push_back({idx[i0], idx[i], idx[i2]});
                idx.erase(idx.begin() + static_cast<std::ptrdiff_t>(i));
                clipped = true;
                break;
            }
            if (!clipped) break;
        }

        if (idx.size() == 3) result.push_back({idx[0], idx[1], idx[2]});
        return result;
    }
};

} // namespace mir
