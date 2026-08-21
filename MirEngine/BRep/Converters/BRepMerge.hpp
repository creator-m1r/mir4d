#pragma once

#include "MirEngine/BRep/Core/BRepModel.hpp"

#include <map>
#include <memory>
#include <vector>

namespace mir
{

[[nodiscard]] inline std::shared_ptr<BRepModel> mergeBRepModels(
    const std::vector<std::shared_ptr<BRepModel>>& sources)
{
    auto merged = std::make_shared<BRepModel>();
    if (sources.empty())
        return merged;

    auto& geo = merged->geometry();
    auto& top = merged->topology();

    const BRepModel* src = nullptr;

    std::map<BRepPointHandle, BRepPointHandle> pointMap;
    std::map<BRepCurveHandle, BRepCurveHandle> curveMap;
    std::map<BRepSurfaceHandle, BRepSurfaceHandle> surfaceMap;
    std::map<BRepVertexHandle, BRepVertexHandle> vertexMap;
    std::map<BRepEdgeHandle, BRepEdgeHandle> edgeMap;
    std::map<BRepWireHandle, BRepWireHandle> wireMap;
    std::map<BRepFaceHandle, BRepFaceHandle> faceMap;
    std::map<BRepShellHandle, BRepShellHandle> shellMap;
    std::map<BRepSolidHandle, BRepSolidHandle> solidMap;

    auto copyPoint = [&](BRepPointHandle h) -> BRepPointHandle
    {
        if (!h.valid())
            return {};
        auto it = pointMap.find(h);
        if (it != pointMap.end())
            return it->second;
        const BRepPointGeometry* s = src->geometry().point(h);
        BRepPointHandle nh = s ? geo.addPoint(*s) : geo.addPoint(BRepPointGeometry{});
        pointMap.emplace(h, nh);
        return nh;
    };

    auto copyCurve = [&](BRepCurveHandle h) -> BRepCurveHandle
    {
        if (!h.valid())
            return {};
        auto it = curveMap.find(h);
        if (it != curveMap.end())
            return it->second;
        const BRepCurveGeometry* s = src->geometry().curve(h);
        BRepCurveGeometry g = s ? *s : BRepCurveGeometry{};
        BRepCurveHandle nh = geo.addCurve(g);
        curveMap.emplace(h, nh);
        return nh;
    };

    auto copySurface = [&](BRepSurfaceHandle h) -> BRepSurfaceHandle
    {
        if (!h.valid())
            return {};
        auto it = surfaceMap.find(h);
        if (it != surfaceMap.end())
            return it->second;
        const BRepSurfaceGeometry* s = src->geometry().surface(h);
        BRepSurfaceGeometry g = s ? *s : BRepSurfaceGeometry{};
        BRepSurfaceHandle nh = geo.addSurface(g);
        surfaceMap.emplace(h, nh);
        return nh;
    };

    auto copyVertex = [&](BRepVertexHandle h) -> BRepVertexHandle
    {
        if (!h.valid())
            return {};
        auto it = vertexMap.find(h);
        if (it != vertexMap.end())
            return it->second;
        const BRepVertex* s = src->topology().vertex(h);
        BRepVertex v = s ? *s : BRepVertex{};
        v.point = copyPoint(v.point);
        v.self = {};
        BRepVertexHandle nh = top.addVertex(v);
        vertexMap.emplace(h, nh);
        return nh;
    };

    auto copyEdge = [&](BRepEdgeHandle h) -> BRepEdgeHandle
    {
        if (!h.valid())
            return {};
        auto it = edgeMap.find(h);
        if (it != edgeMap.end())
            return it->second;
        const BRepEdge* s = src->topology().edge(h);
        BRepEdge e = s ? *s : BRepEdge{};
        e.curve = copyCurve(e.curve);
        e.start = copyVertex(e.start);
        e.end = copyVertex(e.end);
        e.self = {};
        BRepEdgeHandle nh = top.addEdge(e);
        edgeMap.emplace(h, nh);
        return nh;
    };

    auto copyWire = [&](BRepWireHandle h) -> BRepWireHandle
    {
        if (!h.valid())
            return {};
        auto it = wireMap.find(h);
        if (it != wireMap.end())
            return it->second;
        const BRepWire* s = src->topology().wire(h);
        BRepWire w{};
        if (s)
        {
            w.edges.reserve(s->edges.size());
            for (const BRepOrientedEdge& oe : s->edges)
                w.edges.push_back({copyEdge(oe.edge), oe.orientation});
            w.ownerFace = {};
            w.closed = s->closed;
            w.free = s->free;
        }
        BRepWireHandle nh = top.addWire(w);
        wireMap.emplace(h, nh);
        return nh;
    };

    auto copyFace = [&](BRepFaceHandle h) -> BRepFaceHandle
    {
        if (!h.valid())
            return {};
        auto it = faceMap.find(h);
        if (it != faceMap.end())
            return it->second;
        const BRepFace* s = src->topology().face(h);
        BRepFace f{};
        if (s)
        {
            f.surface = copySurface(s->surface);
            f.orientation = s->orientation;
            f.outer = {copyWire(s->outer.wire), s->outer.orientation};
            f.inners.reserve(s->inners.size());
            for (const BRepOrientedWire& ow : s->inners)
                f.inners.push_back({copyWire(ow.wire), ow.orientation});
            f.ownerShell = {};
            f.tolerance = s->tolerance;
            f.naturalRestriction = s->naturalRestriction;
            f.free = s->free;
        }
        BRepFaceHandle nh = top.addFace(f);
        faceMap.emplace(h, nh);
        return nh;
    };

    auto copyShell = [&](BRepShellHandle h) -> BRepShellHandle
    {
        if (!h.valid())
            return {};
        auto it = shellMap.find(h);
        if (it != shellMap.end())
            return it->second;
        const BRepShell* s = src->topology().shell(h);
        BRepShell sh{};
        if (s)
        {
            sh.faces.reserve(s->faces.size());
            for (const BRepOrientedFace& of : s->faces)
                sh.faces.push_back({copyFace(of.face), of.orientation});
            sh.ownerSolid = {};
            sh.closed = s->closed;
            sh.free = s->free;
        }
        BRepShellHandle nh = top.addShell(sh);
        shellMap.emplace(h, nh);
        return nh;
    };

    for (const auto& srcModel : sources)
    {
        if (!srcModel)
            continue;
        src = srcModel.get();
        for (BRepSolidHandle root : srcModel->rootSolids())
        {
            const BRepSolid* s = srcModel->topology().solid(root);
            BRepSolid solid{};
            if (s)
            {
                solid.shells.reserve(s->shells.size());
                for (BRepShellHandle sh : s->shells)
                    solid.shells.push_back(copyShell(sh));
                solid.free = s->free;
            }
            BRepSolidHandle nh = top.addSolid(solid);
            merged->addRootSolid(nh);
            solidMap.emplace(root, nh);
        }
    }

    return merged;
}

}
