#include "BRepStepBridge.hpp"
#include "StepParser.hpp"

#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"
#include "MirEngine/BRep/Topology/BRepTopology.hpp"
#include "MirEngine/Math/Vector/Vector.hpp"
#include "MirEngine/Core/Types/Scalar.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"

#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>

using namespace mir;
using namespace mir::io::step::parser;

namespace
{

constexpr double kPi = 3.14159265358979323846;

Vector3 readCoordList(const Param& list);

void setCoord(const Param& p, double& out)
{
    double v = 0.0;
    if (p.asReal(v))
        out = v;
}

std::string num(double v)
{
    if (!std::isfinite(v))
        v = 0.0;
    std::ostringstream os;
    os << std::setprecision(12) << v;
    return os.str();
}

std::string vecText(const Vector3& v)
{
    return "(" + num(v.x) + "," + num(v.y) + "," + num(v.z) + ")";
}

Vector3 readCoordList(const Param& list)
{
    Vector3 v{};
    if (list.kind != Param::Kind::List)
        return v;
    const std::size_t n = list.items.size();
    if (n >= 1) setCoord(list.items[0], v.x);
    if (n >= 2) setCoord(list.items[1], v.y);
    if (n >= 3) setCoord(list.items[2], v.z);
    return v;
}

bool resolvePoint(
    const Param& p,
    const StepFile& f,
    const std::unordered_map<int, Vector3>& points,
    Vector3& out)
{
    int id = 0;
    if (!p.asReference(id))
        return false;
    auto it = points.find(id);
    if (it != points.end())
    {
        out = it->second;
        return true;
    }
    const Entity* e = f.find(id);
    if (e && e->type == "CARTESIAN_POINT" && e->params.size() >= 2)
        out = readCoordList(e->params[1]);
    return e != nullptr;
}

bool resolveDir(
    const Param& p,
    const StepFile& f,
    const std::unordered_map<int, Vector3>& directions,
    const std::unordered_map<int, Vector3>& vectors,
    Vector3& out)
{
    int id = 0;
    if (!p.asReference(id))
        return false;
    auto it = vectors.find(id);
    if (it != vectors.end())
    {
        out = it->second;
        return true;
    }
    it = directions.find(id);
    if (it != directions.end())
    {
        out = it->second;
        return true;
    }
    const Entity* e = f.find(id);
    if (e && e->type == "VECTOR" && e->params.size() >= 2)
        return resolveDir(e->params[1], f, directions, vectors, out);
    return false;
}

bool resolveVector(
    const Param& p,
    const StepFile& f,
    const std::unordered_map<int, Vector3>& directions,
    const std::unordered_map<int, Vector3>& vectors,
    const std::unordered_map<int, double>& vectorMagnitudes,
    Vector3& outDir,
    double& outMag)
{
    int id = 0;
    if (!p.asReference(id))
        return false;
    auto it = vectorMagnitudes.find(id);
    if (it != vectorMagnitudes.end())
    {
        outMag = it->second;
        auto dit = vectors.find(id);
        if (dit != vectors.end())
            outDir = dit->second;
        else
            resolveDir(p, f, directions, vectors, outDir);
        return true;
    }
    const Entity* e = f.find(id);
    if (e && e->type == "VECTOR" && e->params.size() >= 2)
    {
        if (!resolveDir(e->params[0], f, directions, vectors, outDir))
            return false;
        double mag = 1.0;
        if (e->params.size() >= 2)
            setCoord(e->params[1], mag);
        outMag = mag;
        return true;
    }
    return false;
}

struct Placement
{
    Vector3 location{};
    Vector3 axis{};
    Vector3 ref{};
};

void collectRefs(const Param& p, std::vector<int>& out)
{
    if (p.kind == Param::Kind::List)
    {
        for (const auto& item : p.items)
            collectRefs(item, out);
    }
    else
    {
        int id = 0;
        if (p.asReference(id))
            out.push_back(id);
    }
}

bool isForwardEnum(const Param& p)
{
    return p.kind == Param::Kind::Enum && p.text == ".T.";
}

} // namespace

namespace mir::io::step
{

std::shared_ptr<BRepModel> BRepStepBridge::readFromText(
    const std::string& text,
    std::string& error)
{
    auto file = parse(text);
    if (!file)
    {
        error = "STEP parse failed";
        return nullptr;
    }

    auto model = std::make_shared<BRepModel>();

    std::unordered_map<int, Vector3> points;
    std::unordered_map<int, Vector3> directions;
    std::unordered_map<int, Vector3> vectors;
    std::unordered_map<int, double> vectorMagnitudes;
    std::unordered_map<int, BRepPointHandle> pointHandles;
    std::unordered_map<int, Placement> placements;
    std::unordered_map<int, BRepSurfaceHandle> surfaces;
    std::unordered_map<int, BRepCurveHandle> curves;
    std::unordered_map<int, BRepVertexHandle> vertices;
    std::unordered_map<int, BRepEdgeHandle> edges;
    std::unordered_map<int, BRepWireHandle> wires;
    std::unordered_map<int, BRepFaceHandle> faces;
    std::unordered_map<int, BRepShellHandle> shells;
    std::unordered_map<int, std::pair<int, bool>> orientedEdges;

    // Pass 1a: leaf geometry primitives (points, directions). These do not
    // reference each other, so a single pass is sufficient.
    for (const auto& kv : file->entities)
    {
        const Entity& e = kv.second;
        const std::string& t = e.type;
        if (t == "CARTESIAN_POINT")
        {
            if (e.params.size() >= 2)
            {
                const Vector3 v = readCoordList(e.params[1]);
                points[e.id] = v;
                pointHandles[e.id] = model->geometry().addPoint({v});
            }
        }
        else if (t == "DIRECTION")
        {
            if (e.params.size() >= 2)
                directions[e.id] = readCoordList(e.params[1]);
        }
    }

    // Pass 1b: derived primitives (vectors, placements) that reference the
    // leaf primitives resolved above.
    for (const auto& kv : file->entities)
    {
        const Entity& e = kv.second;
        const std::string& t = e.type;
        if (t == "VECTOR")
        {
            Vector3 d{};
            double mag = 1.0;
            if (e.params.size() >= 2 && resolveDir(e.params[0], *file, directions, vectors, d))
            {
                vectors[e.id] = d;
                if (e.params.size() >= 2)
                    setCoord(e.params[1], mag);
                vectorMagnitudes[e.id] = mag;
            }
        }
        else if (t == "AXIS2_PLACEMENT_3D")
        {
            Placement pl{};
            if (e.params.size() >= 4)
            {
                resolvePoint(e.params[1], *file, points, pl.location);
                resolveDir(e.params[2], *file, directions, vectors, pl.axis);
                resolveDir(e.params[3], *file, directions, vectors, pl.ref);
            }
            placements[e.id] = pl;
        }
    }

    // Pass 2: surfaces and curves.
    for (const auto& kv : file->entities)
    {
        const Entity& e = kv.second;
        const std::string& t = e.type;
        if (t == "PLANE")
        {
            BRepSurfaceGeometry g;
            g.type = BRepSurfaceType::Plane;
            g.uRange = BRepRange{0.0, 1.0};
            g.vRange = BRepRange{0.0, 1.0};
            int aid = 0;
            if (e.params.size() >= 2 && e.params[1].asReference(aid))
            {
                auto it = placements.find(aid);
                if (it != placements.end())
                {
                    g.plane.location = it->second.location;
                    g.plane.normal = it->second.axis;
                    g.plane.xDir = it->second.ref;
                }
            }
            surfaces[e.id] = model->geometry().addSurface(g);
        }
        else if (t == "CYLINDRICAL_SURFACE")
        {
            BRepSurfaceGeometry g;
            g.type = BRepSurfaceType::Cylinder;
            g.uRange = BRepRange{0.0, 2.0 * kPi};
            g.vRange = BRepRange{0.0, 1.0};
            int aid = 0;
            if (e.params.size() >= 2 && e.params[1].asReference(aid))
            {
                auto it = placements.find(aid);
                if (it != placements.end())
                {
                    g.cylinder.location = it->second.location;
                    g.cylinder.axis = it->second.axis;
                    g.cylinder.xDir = it->second.ref;
                }
            }
            double r = 1.0;
            if (e.params.size() >= 3)
                setCoord(e.params[2], r);
            g.cylinder.radius = r;
            surfaces[e.id] = model->geometry().addSurface(g);
        }
        else if (t == "LINE")
        {
            BRepCurveGeometry g;
            g.type = BRepCurveType::Line;
            g.range = BRepRange{0.0, 1.0};
            Vector3 loc{};
            if (e.params.size() >= 2)
                resolvePoint(e.params[1], *file, points, loc);
            Vector3 dir{};
            double mag = 1.0;
            if (e.params.size() >= 3)
                resolveVector(e.params[2], *file, directions, vectors, vectorMagnitudes, dir, mag);
            if (dir.isZero())
                dir = Vector3::unitZ();
            g.line.location = loc;
            g.line.direction = dir * mag;
            curves[e.id] = model->geometry().addCurve(g);
        }
        else if (t == "CIRCLE")
        {
            BRepCurveGeometry g;
            g.type = BRepCurveType::Circle;
            g.range = BRepRange{0.0, 2.0 * kPi};
            int aid = 0;
            if (e.params.size() >= 2 && e.params[1].asReference(aid))
            {
                auto it = placements.find(aid);
                if (it != placements.end())
                {
                    g.circle.center = it->second.location;
                    g.circle.normal = it->second.axis;
                    g.circle.xDir = it->second.ref;
                }
            }
            double r = 1.0;
            if (e.params.size() >= 3)
                setCoord(e.params[2], r);
            g.circle.radius = r;
            curves[e.id] = model->geometry().addCurve(g);
        }
    }

    // Pass 3: topology, in dependency order so referenced entities exist
    // before their references are resolved.
    auto eachOf = [&](const char* type, auto&& fn)
    {
        for (const auto& kv : file->entities)
        {
            if (kv.second.type != type)
                continue;
            fn(kv.second);
        }
    };

    eachOf("VERTEX_POINT", [&](const Entity& e)
    {
        if (e.params.size() < 2)
            return;
        int pid = 0;
        if (!e.params[1].asReference(pid))
            return;
        auto it = pointHandles.find(pid);
        if (it == pointHandles.end())
            return;
        BRepVertex v{};
        v.point = it->second;
        vertices[e.id] = model->topology().addVertex(v);
    });

    eachOf("EDGE_CURVE", [&](const Entity& e)
    {
        if (e.params.size() < 4)
            return;
        int v1 = 0, v2 = 0, cid = 0;
        if (!e.params[1].asReference(v1) || !e.params[2].asReference(v2) ||
            !e.params[3].asReference(cid))
            return;
        auto vit1 = vertices.find(v1);
        auto vit2 = vertices.find(v2);
        auto cit = curves.find(cid);
        if (vit1 == vertices.end() || vit2 == vertices.end() || cit == curves.end())
            return;
        BRepEdge edge{};
        edge.curve = cit->second;
        edge.start = vit1->second;
        edge.end = vit2->second;
        if (const BRepCurveGeometry* cg = model->geometry().curve(edge.curve))
            edge.range = cg->range;
        edges[e.id] = model->topology().addEdge(edge);
    });

    eachOf("ORIENTED_EDGE", [&](const Entity& e)
    {
        if (e.params.size() < 3)
            return;
        int eid = 0;
        if (e.params[2].asReference(eid))
            orientedEdges[e.id] = {eid, isForwardEnum(e.params[3])};
    });

    eachOf("EDGE_LOOP", [&](const Entity& e)
    {
        BRepWire wire{};
        std::vector<int> refs;
        if (e.params.size() >= 2)
            collectRefs(e.params[1], refs);
        for (int rid : refs)
        {
            auto it = orientedEdges.find(rid);
            if (it == orientedEdges.end())
                continue;
            auto edgeIt = edges.find(it->second.first);
            if (edgeIt == edges.end())
                continue;
            BRepOrientedEdge oe{};
            oe.edge = edgeIt->second;
            oe.orientation = it->second.second ? BRepOrientation::Forward
                                              : BRepOrientation::Reversed;
            wire.edges.push_back(oe);
        }
        if (!wire.edges.empty())
            wires[e.id] = model->topology().addWire(wire);
    });

    eachOf("FACE_SURFACE", [&](const Entity& e)
    {
        BRepFace face{};
        int sid = 0;
        if (e.params.size() >= 2 && e.params[1].asReference(sid))
        {
            auto sit = surfaces.find(sid);
            if (sit != surfaces.end())
                face.surface = sit->second;
        }

        std::vector<int> boundRefs;
        for (std::size_t i = 3; i < e.params.size(); ++i)
            collectRefs(e.params[i], boundRefs);

        bool first = true;
        for (int brid : boundRefs)
        {
            const Entity* be = file->find(brid);
            if (!be || be->type != "FACE_BOUND")
                continue;
            int lid = 0;
            if (be->params.size() < 2 || !be->params[1].asReference(lid))
                continue;
            auto wit = wires.find(lid);
            if (wit == wires.end())
                continue;
            const bool forward = be->params.size() >= 3
                ? isForwardEnum(be->params[2])
                : true;
            if (first)
            {
                face.outer.wire = wit->second;
                face.outer.orientation = forward ? BRepOrientation::Forward
                                                : BRepOrientation::Reversed;
                first = false;
            }
            else
            {
                BRepOrientedWire inner{};
                inner.wire = wit->second;
                inner.orientation = forward ? BRepOrientation::Forward
                                            : BRepOrientation::Reversed;
                face.inners.push_back(inner);
            }
        }
        if (face.surface.valid() && face.outer.valid())
            faces[e.id] = model->topology().addFace(face);
    });

    auto addShell = [&](const Entity& e)
    {
        BRepShell shell{};
        std::vector<int> refs;
        if (e.params.size() >= 2)
            collectRefs(e.params[1], refs);
        for (int fid : refs)
        {
            auto fit = faces.find(fid);
            if (fit == faces.end())
                continue;
            BRepOrientedFace of{};
            of.face = fit->second;
            of.orientation = BRepOrientation::Forward;
            shell.faces.push_back(of);
        }
        if (!shell.faces.empty())
            shells[e.id] = model->topology().addShell(shell);
    };
    eachOf("CLOSED_SHELL", addShell);
    eachOf("OPEN_SHELL", addShell);
    eachOf("SHELL_BASED_SURFACE_MODEL", addShell);

    eachOf("MANIFOLD_SOLID_BREP", [&](const Entity& e)
    {
        BRepSolid solid{};
        std::vector<int> shellRefs;
        if (e.params.size() >= 2)
            collectRefs(e.params[1], shellRefs);
        for (int shid : shellRefs)
        {
            auto sit = shells.find(shid);
            if (sit == shells.end())
                continue;
            solid.shells.push_back(sit->second);
        }
        if (!solid.shells.empty())
        {
            const BRepSolidHandle handle = model->topology().addSolid(solid);
            model->addRootSolid(handle);
        }
    });
    eachOf("BREP_WITH_VOIDS", [&](const Entity& e)
    {
        BRepSolid solid{};
        std::vector<int> shellRefs;
        if (e.params.size() >= 2)
            collectRefs(e.params[1], shellRefs);
        for (int shid : shellRefs)
        {
            auto sit = shells.find(shid);
            if (sit == shells.end())
                continue;
            solid.shells.push_back(sit->second);
        }
        if (!solid.shells.empty())
        {
            const BRepSolidHandle handle = model->topology().addSolid(solid);
            model->addRootSolid(handle);
        }
    });

    if (model->rootSolids().empty())
    {
        error = "STEP file contains no supported B-Rep solids";
        return nullptr;
    }

    error.clear();
    return model;
}

std::shared_ptr<BRepModel> BRepStepBridge::read(
    const std::string& path,
    std::string& error)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
    {
        error = "Cannot open STEP file: " + path;
        return nullptr;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return readFromText(ss.str(), error);
}

bool BRepStepBridge::writeToText(
    std::string& out,
    const BRepModel& model,
    std::string& error)
{
    std::ostringstream header;
    header << "ISO-10303-21;\n";
    header << "HEADER;\n";
    header << "FILE_DESCRIPTION(('MIR 4D native B-Rep'),'2;1');\n";
    header << "FILE_NAME('MIR4D.BREP','',('MIR 4D'),('MIR 4D'),'','','','');\n";
    header << "FILE_SCHEMA(('CONFIG_CONTROL_DESIGN'));\n";
    header << "ENDSEC;\n";
    header << "DATA;\n";

    std::vector<std::string> body;
    int nextId = 14;

    std::unordered_map<BRepPointHandle, int> pointIds;
    std::unordered_map<BRepCurveHandle, int> curveIds;
    std::unordered_map<BRepSurfaceHandle, int> surfaceIds;
    std::unordered_map<BRepVertexHandle, int> vertexIds;
    std::unordered_map<BRepEdgeHandle, int> edgeIds;
    std::unordered_map<BRepFaceHandle, int> faceIds;
    std::unordered_map<BRepShellHandle, int> shellIds;

    std::function<int(const Vector3&, const Vector3&, const Vector3&)> emitAxis2 =
        [&](const Vector3& loc, const Vector3& axis, const Vector3& ref) -> int
    {
        const int pid = nextId++;
        const int poid = nextId++;
        const int ad = nextId++;
        const int rd = nextId++;
        body.push_back("#" + std::to_string(poid) + "=CARTESIAN_POINT(''," + vecText(loc) + ");");
        body.push_back("#" + std::to_string(ad) + "=DIRECTION(''," + vecText(axis.normalized()) + ");");
        body.push_back("#" + std::to_string(rd) + "=DIRECTION(''," + vecText(ref.normalized()) + ");");
        body.push_back("#" + std::to_string(pid) + "=AXIS2_PLACEMENT_3D('',#" +
                       std::to_string(poid) + ",#" + std::to_string(ad) + ",#" +
                       std::to_string(rd) + ");");
        return pid;
    };

    std::function<int(BRepSurfaceHandle)> ensureSurface =
        [&](BRepSurfaceHandle h) -> int
    {
        auto it = surfaceIds.find(h);
        if (it != surfaceIds.end())
            return it->second;
        if (!h.valid())
            return 0;
        const BRepSurfaceGeometry* surf = model.geometry().surface(h);
        if (!surf)
            return 0;
        const int id = nextId++;
        surfaceIds.emplace(h, id);
        if (surf->type == BRepSurfaceType::Plane)
        {
            const int ax = emitAxis2(surf->plane.location, surf->plane.normal, surf->plane.xDir);
            body.push_back("#" + std::to_string(id) + "=PLANE('',#" + std::to_string(ax) + ");");
        }
        else if (surf->type == BRepSurfaceType::Cylinder)
        {
            const int ax = emitAxis2(surf->cylinder.location, surf->cylinder.axis, surf->cylinder.xDir);
            body.push_back("#" + std::to_string(id) + "=CYLINDRICAL_SURFACE('',#" +
                           std::to_string(ax) + "," + num(surf->cylinder.radius) + ");");
        }
        return id;
    };

    std::function<int(BRepCurveHandle)> ensureCurve =
        [&](BRepCurveHandle h) -> int
    {
        auto it = curveIds.find(h);
        if (it != curveIds.end())
            return it->second;
        if (!h.valid())
            return 0;
        const BRepCurveGeometry* cg = model.geometry().curve(h);
        if (!cg)
            return 0;
        const int id = nextId++;
        curveIds.emplace(h, id);
        if (cg->type == BRepCurveType::Line)
        {
            const int lp = nextId++;
            const int ld = nextId++;
            const int lv = nextId++;
            const Vector3 dirN = cg->line.direction.normalized();
            const double mag = cg->line.direction.length() * cg->range.length();
            body.push_back("#" + std::to_string(lp) + "=CARTESIAN_POINT(''," + vecText(cg->line.location) + ");");
            body.push_back("#" + std::to_string(ld) + "=DIRECTION(''," + vecText(dirN) + ");");
            body.push_back("#" + std::to_string(lv) + "=VECTOR(#" + std::to_string(ld) + "," + num(mag) + ");");
            body.push_back("#" + std::to_string(id) + "=LINE('',#" + std::to_string(lp) +
                           ",#" + std::to_string(lv) + ");");
        }
        else if (cg->type == BRepCurveType::Circle || cg->type == BRepCurveType::Arc)
        {
            const int ax = emitAxis2(cg->circle.center, cg->circle.normal, cg->circle.xDir);
            body.push_back("#" + std::to_string(id) + "=CIRCLE('',#" + std::to_string(ax) +
                           "," + num(cg->circle.radius) + ");");
        }
        return id;
    };

    std::function<int(BRepVertexHandle)> ensureVertex =
        [&](BRepVertexHandle h) -> int
    {
        auto it = vertexIds.find(h);
        if (it != vertexIds.end())
            return it->second;
        if (!h.valid())
            return 0;
        const BRepVertex* v = model.topology().vertex(h);
        if (!v)
            return 0;
        const BRepPointGeometry* p = model.geometry().point(v->point);
        if (!p)
            return 0;
        const int id = nextId++;
        vertexIds.emplace(h, id);
        const int pid = nextId++;
        body.push_back("#" + std::to_string(pid) + "=CARTESIAN_POINT(''," + vecText(p->point) + ");");
        body.push_back("#" + std::to_string(id) + "=VERTEX_POINT('',#" + std::to_string(pid) + ");");
        return id;
    };

    std::function<int(BRepEdgeHandle)> ensureEdge =
        [&](BRepEdgeHandle h) -> int
    {
        auto it = edgeIds.find(h);
        if (it != edgeIds.end())
            return it->second;
        if (!h.valid())
            return 0;
        const BRepEdge* edge = model.topology().edge(h);
        if (!edge)
            return 0;
        const int curveId = ensureCurve(edge->curve);
        const int v1Id = ensureVertex(edge->start);
        const int v2Id = ensureVertex(edge->end);
        if (curveId == 0 || v1Id == 0 || v2Id == 0)
            return 0;
        const int id = nextId++;
        edgeIds.emplace(h, id);
        body.push_back("#" + std::to_string(id) + "=EDGE_CURVE('',#" + std::to_string(v1Id) +
                       ",#" + std::to_string(v2Id) + ",#" + std::to_string(curveId) + ",.T.);");
        return id;
    };

    std::function<int(BRepWireHandle, bool)> emitLoop =
        [&](BRepWireHandle wireHandle, bool outer) -> int
    {
        const BRepWire* wire = model.topology().wire(wireHandle);
        if (!wire || wire->edges.empty())
            return 0;
        std::vector<int> oeIds;
        for (const BRepOrientedEdge& oe : wire->edges)
        {
            const int edgeId = ensureEdge(oe.edge);
            if (edgeId == 0)
                continue;
            const int oeId = nextId++;
            body.push_back("#" + std::to_string(oeId) + "=ORIENTED_EDGE('',*,#" +
                           std::to_string(edgeId) + "," +
                           (isForward(oe.orientation) ? ".T." : ".F.") + ");");
            oeIds.push_back(oeId);
        }
        if (oeIds.empty())
            return 0;
        const int loopId = nextId++;
        std::string list = "(";
        for (std::size_t i = 0; i < oeIds.size(); ++i)
        {
            if (i > 0) list += ",";
            list += "#" + std::to_string(oeIds[i]);
        }
        list += ")";
        body.push_back("#" + std::to_string(loopId) + "=EDGE_LOOP(''," + list + ");");

        const int boundId = nextId++;
        body.push_back("#" + std::to_string(boundId) + "=FACE_BOUND('',#" +
                       std::to_string(loopId) + "," + (outer ? ".T." : ".F.") + ");");
        return boundId;
    };

    std::function<int(BRepFaceHandle)> ensureFace =
        [&](BRepFaceHandle h) -> int
    {
        auto it = faceIds.find(h);
        if (it != faceIds.end())
            return it->second;
        if (!h.valid())
            return 0;
        const BRepFace* face = model.topology().face(h);
        if (!face)
            return 0;
        const int surfaceId = ensureSurface(face->surface);
        if (surfaceId == 0)
            return 0;

        std::vector<int> boundIds;
        const int outerBound = emitLoop(face->outer.wire, true);
        if (outerBound == 0)
            return 0;
        boundIds.push_back(outerBound);
        for (const BRepOrientedWire& inner : face->inners)
        {
            const int innerBound = emitLoop(inner.wire, false);
            if (innerBound != 0)
                boundIds.push_back(innerBound);
        }

        const int id = nextId++;
        faceIds.emplace(h, id);
        std::string list = "(";
        for (std::size_t i = 0; i < boundIds.size(); ++i)
        {
            if (i > 0) list += ",";
            list += "#" + std::to_string(boundIds[i]);
        }
        list += ")";
        body.push_back("#" + std::to_string(id) + "=FACE_SURFACE('',#" +
                       std::to_string(surfaceId) + ",.T.," + list + ");");
        return id;
    };

    std::vector<int> manifoldIds;

    for (const BRepSolidHandle solidHandle : model.rootSolids())
    {
        const BRepSolid* solid = model.topology().solid(solidHandle);
        if (!solid)
            continue;

        for (const BRepShellHandle shellHandle : solid->shells)
        {
            const BRepShell* shell = model.topology().shell(shellHandle);
            if (!shell)
                continue;

            std::vector<int> faceIdList;
            for (const BRepOrientedFace& orientedFace : shell->faces)
            {
                const int fid = ensureFace(orientedFace.face);
                if (fid != 0)
                    faceIdList.push_back(fid);
            }
            if (faceIdList.empty())
                continue;

            const int shellId = nextId++;
            std::string list = "(";
            for (std::size_t i = 0; i < faceIdList.size(); ++i)
            {
                if (i > 0) list += ",";
                list += "#" + std::to_string(faceIdList[i]);
            }
            list += ")";
            body.push_back("#" + std::to_string(shellId) + "=CLOSED_SHELL(''," + list + ");");
            shellIds.emplace(shellHandle, shellId);

            const int mid = nextId++;
            body.push_back("#" + std::to_string(mid) +
                           "=MANIFOLD_SOLID_BREP('MIR4D',#" + std::to_string(shellId) + ");");
            manifoldIds.push_back(mid);
        }
    }

    if (manifoldIds.empty())
    {
        error = "BRepModel contains no solids to export";
        return false;
    }

    std::ostringstream boiler;
    boiler << "#1=APPLICATION_CONTEXT('automotive design');\n";
    boiler << "#2=APPLICATION_PROTOCOL_DEFINITION('international standard','config_control_design',2000,#1);\n";
    boiler << "#3=( LENGTH_UNIT ( ) NAMED_UNIT ( * ) SI_UNIT ( $.MILLI. , .METRE. ) );\n";
    boiler << "#4=UNCERTAINTY_MEASURE_WITH_UNIT(LENGTH_MEASURE(1.0E-3),#3,'distance_accuracy','');\n";
    boiler << "#5=( REPRESENTATION_CONTEXT ( 'NONE','3D' ) GEOMETRIC_REPRESENTATION_CONTEXT ( 3 ) GLOBAL_UNCERTAINTY_ASSIGNMENT ( ( #4 ) ) );\n";
    boiler << "#6=PRODUCT('MIR4D_PART','MIR 4D B-Rep model','',(#7));\n";
    boiler << "#7=PRODUCT_CONTEXT('',#1,'mechanical');\n";
    boiler << "#8=PRODUCT_DEFINITION_FORMATION('','',#6);\n";
    boiler << "#9=PRODUCT_DEFINITION('design','',#8,#7);\n";
    boiler << "#10=PRODUCT_DEFINITION_CONTEXT('part definition',#1,'design');\n";
    boiler << "#11=PRODUCT_DEFINITION_SHAPE('','',#9);\n";
    std::string repList = "(";
    for (std::size_t i = 0; i < manifoldIds.size(); ++i)
    {
        if (i > 0) repList += ",";
        repList += "#" + std::to_string(manifoldIds[i]);
    }
    repList += ")";
    boiler << "#12=ADVANCED_BREP_SHAPE_REPRESENTATION('',(" << repList << ",#5),#4);\n";
    boiler << "#13=SHAPE_DEFINITION_REPRESENTATION(#11,#12);\n";

    std::ostringstream full;
    full << header.str();
    full << boiler.str();
    for (const auto& line : body)
        full << line << "\n";
    full << "ENDSEC;\n";
    full << "END-ISO-10303-21;\n";

    out = full.str();
    error.clear();
    return true;
}

bool BRepStepBridge::write(
    const std::string& path,
    const BRepModel& model,
    std::string& error)
{
    std::string text;
    if (!writeToText(text, model, error))
        return false;
    std::ofstream out(path, std::ios::binary);
    if (!out)
    {
        error = "Cannot write STEP file: " + path;
        return false;
    }
    out << text;
    error.clear();
    return true;
}

} // namespace mir::io::step
