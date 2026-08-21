#include "StepImporter.hpp"
#include "StepParser.hpp"

#include "../../Geometry/Tessellation/TriangleMesh.hpp"
#include "../../Math/Point.hpp"
#include "../../Math/Vector/Vector.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mir::io::step
{

namespace
{

using parser::Entity;
using parser::Param;
using parser::StepFile;

Point3 toPoint3(const std::vector<Param>& coords)
{
    double x = 0.0, y = 0.0, z = 0.0;
    if (coords.size() > 0) (void)coords[0].asReal(x);
    if (coords.size() > 1) (void)coords[1].asReal(y);
    if (coords.size() > 2) (void)coords[2].asReal(z);
    return {x, y, z};
}

const std::vector<Param>* firstList(const Entity& entity)
{
    for (const Param& p : entity.params)
        if (p.kind == Param::Kind::List)
            return &p.items;
    return nullptr;
}

struct Accumulator
{
    TriangleMesh3 mesh;
    std::unordered_map<std::uint64_t, std::size_t> vertexMap;
    std::vector<Vector3> normalSums;

    static std::uint64_t key(double x, double y, double z)
    {
        std::uint64_t bx, by, bz;
        std::memcpy(&bx, &x, sizeof(bx));
        std::memcpy(&by, &y, sizeof(by));
        std::memcpy(&bz, &z, sizeof(bz));
        return bx ^ (by << 1) ^ (bz << 2);
    }

    std::size_t vertex(const Point3& p)
    {
        const std::uint64_t k = key(p.x, p.y, p.z);
        auto found = vertexMap.find(k);
        if (found != vertexMap.end())
            return found->second;
        const std::size_t index = mesh.vertices.size();
        mesh.vertices.push_back(p);
        normalSums.emplace_back(0.0, 0.0, 0.0);
        vertexMap.emplace(k, index);
        return index;
    }

    void addTriangle(const Point3& a, const Point3& b, const Point3& c)
    {
        const std::size_t ia = vertex(a);
        const std::size_t ib = vertex(b);
        const std::size_t ic = vertex(c);
        if (ia == ib || ib == ic || ia == ic)
            return;
        mesh.triangles.push_back({ia, ib, ic});
        const Vector3 ab{b.x - a.x, b.y - a.y, b.z - a.z};
        const Vector3 ac{c.x - a.x, c.y - a.y, c.z - a.z};
        Vector3 n = Vector3::cross(ab, ac);
        const double len = n.length();
        if (len > 1e-18)
            n = n * (1.0 / len);
        else
            n = Vector3{0.0, 0.0, 1.0};
        if (normalSums.size() != mesh.vertices.size())
            normalSums.resize(mesh.vertices.size(), Vector3{0.0, 0.0, 0.0});
        normalSums[ia] = normalSums[ia] + n;
        normalSums[ib] = normalSums[ib] + n;
        normalSums[ic] = normalSums[ic] + n;
    }

    void finalizeNormals()
    {
        mesh.normals.resize(mesh.vertices.size());
        for (std::size_t i = 0; i < normalSums.size(); ++i)
        {
            const Vector3 n = normalSums[i];
            mesh.normals[i] = n.length() > 1e-18
                ? n.normalized()
                : Vector3{0.0, 0.0, 1.0};
        }
    }
};

void emitPolygon(Accumulator& acc, const std::vector<Point3>& points)
{
    if (points.size() < 3)
        return;
    for (std::size_t i = 1; i + 1 < points.size(); ++i)
        acc.addTriangle(points[0], points[i], points[i + 1]);
}

void collectPolyLoopPoints(const StepFile& file,
                           int loopId,
                           const std::unordered_map<int, Point3>& points,
                           std::vector<Point3>& out)
{
    const Entity* loop = file.find(loopId);
    if (loop == nullptr)
        return;
    if (loop->type != "POLY_LOOP")
        return;
    if (loop->params.size() < 2 || loop->params[1].kind != Param::Kind::List)
        return;
    for (const Param& ref : loop->params[1].items)
    {
        int pid = 0;
        if (ref.asReference(pid))
        {
            auto it = points.find(pid);
            if (it != points.end())
                out.push_back(it->second);
        }
    }
}

void collectFace(const StepFile& file,
                 int faceId,
                 const std::unordered_map<int, Point3>& points,
                 Accumulator& acc)
{
    const Entity* face = file.find(faceId);
    if (face == nullptr)
        return;
    if (face->type != "FACE" && face->type != "ADVANCED_FACE")
        return;

    const std::vector<Param>* bounds = firstList(*face);
    if (bounds == nullptr)
        return;

    for (const Param& boundRef : *bounds)
    {
        int boundId = 0;
        if (!boundRef.asReference(boundId))
            continue;
        const Entity* bound = file.find(boundId);
        if (bound == nullptr)
            continue;
        if (bound->type != "FACE_BOUND" && bound->type != "FACE_OUTER_BOUND")
            continue;
        if (bound->params.empty())
            continue;
        int loopId = 0;
        if (!bound->params[0].asReference(loopId))
            continue;
        std::vector<Point3> polygon;
        collectPolyLoopPoints(file, loopId, points, polygon);
        emitPolygon(acc, polygon);
    }
}

void collectShell(const StepFile& file,
                  int shellId,
                  const std::unordered_map<int, Point3>& points,
                  Accumulator& acc,
                  std::unordered_set<int>& visited)
{
    if (visited.count(shellId) != 0)
        return;
    visited.insert(shellId);

    const Entity* shell = file.find(shellId);
    if (shell == nullptr)
        return;
    if (shell->params.size() < 2 || shell->params[1].kind != Param::Kind::List)
        return;
    for (const Param& faceRef : shell->params[1].items)
    {
        int faceId = 0;
        if (faceRef.asReference(faceId))
            collectFace(file, faceId, points, acc);
    }
}

void collectTessellatedFace(const StepFile& file,
                            int faceId,
                            const std::vector<Point3>& coordinates,
                            const std::unordered_map<int, Point3>& points,
                            Accumulator& acc)
{
    const Entity* face = file.find(faceId);
    if (face == nullptr)
        return;

    if (face->type == "TRIANGULATED_FACE")
    {

        std::vector<Param> triangles;
        for (const Param& p : face->params)
        {
            if (p.kind == Param::Kind::List && !p.items.empty() &&
                p.items.front().kind == Param::Kind::List &&
                p.items.front().items.size() == 3 &&
                p.items.front().items.front().kind == Param::Kind::Integer)
            {
                triangles.push_back(p);
            }
        }
        for (const Param& tri : triangles)
        {
            std::array<Point3, 3> verts{};
            bool ok = true;
            for (int i = 0; i < 3; ++i)
            {
                int idx = 0;
                double dv = 0.0;
                if (tri.items[i].asReference(idx))
                {

                }
                else if (tri.items[i].asReal(dv))
                {
                    idx = static_cast<int>(dv);
                }
                else
                {
                    ok = false;
                    break;
                }
                if (idx < 1 || static_cast<std::size_t>(idx) > coordinates.size())
                {
                    ok = false;
                    break;
                }
                verts[i] = coordinates[static_cast<std::size_t>(idx) - 1];
            }
            if (ok)
                acc.addTriangle(verts[0], verts[1], verts[2]);
        }
        return;
    }

    if (face->type == "POLYGONAL_FACE")
    {
        for (const Param& bounds : face->params)
        {
            if (bounds.kind != Param::Kind::List)
                continue;
            for (const Param& boundRef : bounds.items)
            {
                int boundId = 0;
                if (!boundRef.asReference(boundId))
                    continue;
                const Entity* bound = file.find(boundId);
                if (bound == nullptr || bound->type != "POLYGONAL_BOUND")
                    continue;
                if (bound->params.size() < 2 || bound->params[1].kind != Param::Kind::List)
                    continue;
                std::vector<Point3> polygon;
                for (const Param& ptRef : bound->params[1].items)
                {
                    int pid = 0;
                    if (ptRef.asReference(pid))
                    {
                        auto it = points.find(pid);
                        if (it != points.end())
                            polygon.push_back(it->second);
                    }
                }
                emitPolygon(acc, polygon);
            }
        }
    }
}

int resolveShellFromBrep(const Entity& brep)
{

    if (brep.params.size() < 2)
        return 0;
    int shellId = 0;
    if (brep.params[1].asReference(shellId))
        return shellId;
    return 0;
}

int resolveBrepFromTools(const Entity& tools)
{
    if (tools.params.size() < 2)
        return 0;
    int brepId = 0;
    if (tools.params[1].asReference(brepId))
        return brepId;
    return 0;
}

}

ImportResult StepImporter::importFile(
    const std::string& path,
    const ImportOptions&) const
{
    ImportResult result;
    result.format = Format::Step;
    result.sourcePath = path;

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        result.error = "Cannot open STEP file: " + path;
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    const std::string text = buffer.str();
    if (text.empty())
    {
        result.error = "STEP file is empty: " + path;
        return result;
    }

    auto stepFile = parser::parse(text);
    if (!stepFile)
    {
        result.error = "Failed to parse STEP file: " + path;
        return result;
    }

    std::unordered_map<int, Point3> points;
    for (const auto& pair : stepFile->entities)
    {
        const Entity& entity = pair.second;
        if (entity.type != "CARTESIAN_POINT")
            continue;
        if (entity.params.size() < 2 || entity.params[1].kind != Param::Kind::List)
            continue;
        points.emplace(entity.id, toPoint3(entity.params[1].items));
    }

    Accumulator acc;
    std::unordered_set<int> visitedShells;

    for (const auto& pair : stepFile->entities)
    {
        const Entity& entity = pair.second;
        const std::string& type = entity.type;

        if (type == "FACETED_BREP" || type == "MANIFOLD_SOLID_BREP" ||
            type == "SHELL_BASED_SURFACE_MODEL")
        {
            const int shellId = resolveShellFromBrep(entity);
            if (shellId != 0)
                collectShell(*stepFile, shellId, points, acc, visitedShells);
        }
        else if (type == "BREP_WITH_TOOLS")
        {
            const int brepId = resolveBrepFromTools(entity);
            const Entity* brep = stepFile->find(brepId);
            if (brep != nullptr)
            {
                const int shellId = resolveShellFromBrep(*brep);
                if (shellId != 0)
                    collectShell(*stepFile, shellId, points, acc, visitedShells);
            }
        }
        else if (type == "CLOSED_SHELL" || type == "OPEN_SHELL")
        {
            collectShell(*stepFile, entity.id, points, acc, visitedShells);
        }
        else if (type == "TESSELLATED_SHELL")
        {

            std::vector<Point3> coordinates;
            const std::vector<Param>* coords = nullptr;
            const std::vector<Param>* faces = nullptr;
            int listIndex = 0;
            for (const Param& p : entity.params)
            {
                if (p.kind == Param::Kind::List)
                {
                    if (listIndex == 0)
                        coords = &p.items;
                    else if (listIndex == 1)
                        faces = &p.items;
                    ++listIndex;
                }
            }
            if (coords != nullptr)
            {
                for (const Param& c : *coords)
                {
                    if (c.kind == Param::Kind::List)
                        coordinates.push_back(toPoint3(c.items));
                }
            }
            if (faces != nullptr)
            {
                for (const Param& faceRef : *faces)
                {
                    int faceId = 0;
                    if (faceRef.asReference(faceId))
                        collectTessellatedFace(*stepFile, faceId, coordinates, points, acc);
                }
            }
        }
    }

    acc.finalizeNormals();

    if (!acc.mesh.isValid())
    {
        result.error = "STEP file contains no supported geometry (faceted_brep / "
                       "tessellated_shell). B-Rep-to-mesh mapping requires the "
                       "optional OpenCASCADE bridge.";
        return result;
    }

    result.mesh = std::make_shared<TriangleMesh3>(std::move(acc.mesh));
    result.boundsMin = result.mesh->boundsMin();
    result.boundsMax = result.mesh->boundsMax();
    result.triangleCount = result.mesh->triangles.size();
    return result;
}

}
