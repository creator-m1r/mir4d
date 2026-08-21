#pragma once

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"
#include "MirEngine/BRep/Topology/BRepTopologyEditor.hpp"

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace mir
{

class BRepBuilderAPI
{
public:
    explicit BRepBuilderAPI(BRepModel& model) noexcept
        : model_(model)
    {
    }

    [[nodiscard]] BRepVertexHandle makeVertex(
        const Vector3& point,
        double tolerance = DefaultBRepTolerance.linear)
    {
        if (!point.isFinite() || !(tolerance > 0.0))
            return {};

        BRepPointGeometry pointGeometry{};
        pointGeometry.point = point;

        BRepVertex vertex{};
        vertex.point = model_.geometry().addPoint(std::move(pointGeometry));
        vertex.tolerance = tolerance;
        vertex.free = false;
        return model_.topology().addVertex(std::move(vertex));
    }

    [[nodiscard]] BRepEdgeHandle makeEdgeLine(
        BRepVertexHandle start,
        BRepVertexHandle end,
        double tolerance = DefaultBRepTolerance.linear)
    {
        const BRepVertex* startVertex = model_.topology().vertex(start);
        const BRepVertex* endVertex = model_.topology().vertex(end);
        if (!startVertex || !endVertex || !(tolerance > 0.0))
            return {};

        const BRepPointGeometry* p0 = model_.geometry().point(startVertex->point);
        const BRepPointGeometry* p1 = model_.geometry().point(endVertex->point);
        if (!p0 || !p1)
            return {};

        const Vector3 a = p0->point;
        const Vector3 b = p1->point;
        const Vector3 delta = b - a;
        const Scalar length = delta.length();
        if (!(length > Scalar(0.0)) || !std::isfinite(length))
            return {};

        BRepCurveGeometry curve{};
        curve.type = BRepCurveType::Line;
        curve.range = {0.0, length};
        curve.line.location = a;
        curve.line.direction = delta / length;

        BRepEdge edge{};
        edge.curve = model_.geometry().addCurve(std::move(curve));
        edge.range = {0.0, length};
        edge.start = start;
        edge.end = end;
        edge.tolerance = tolerance;
        edge.free = false;
        return model_.topology().addEdge(std::move(edge));
    }

    [[nodiscard]] BRepEdgeHandle makeEdgeFromCurve(
        BRepCurveGeometry curveGeometry,
        BRepVertexHandle start,
        BRepVertexHandle end,
        double tolerance = DefaultBRepTolerance.linear)
    {
        if (!curveGeometry.isValid() || !(tolerance > 0.0))
            return {};
        if (!model_.topology().vertex(start) || !model_.topology().vertex(end))
            return {};

        BRepEdge edge{};
        edge.range = curveGeometry.range;
        edge.curve = model_.geometry().addCurve(std::move(curveGeometry));
        edge.start = start;
        edge.end = end;
        edge.tolerance = tolerance;
        edge.free = false;
        return model_.topology().addEdge(std::move(edge));
    }

    [[nodiscard]] BRepWireHandle makeWire(
        const std::vector<BRepOrientedEdge>& orientedEdges,
        bool closed)
    {
        if (orientedEdges.empty())
            return {};

        if (!edgesCanFormWire(orientedEdges, closed))
            return {};

        const auto checkpoint = model_.checkpoint();
        BRepWire wire{};
        wire.closed = false;
        wire.free = false;
        const BRepWireHandle handle = model_.topology().addWire(std::move(wire));

        for (const BRepOrientedEdge& item : orientedEdges)
        {
            if (!BRepTopologyEditor::addEdgeToWire(model_.topology(), handle, item))
            {
                model_.rollback(checkpoint);
                return {};
            }
        }

        if (closed && !BRepTopologyEditor::markWireClosed(model_.topology(), handle, true))
        {
            model_.rollback(checkpoint);
            return {};
        }

        return handle;
    }

    [[nodiscard]] BRepWireHandle makeWireFromEdges(
        const std::vector<BRepEdgeHandle>& edges,
        bool closed)
    {
        if (edges.size() < 2)
            return {};

        std::vector<BRepOrientedEdge> oriented;
        oriented.reserve(edges.size());
        for (BRepEdgeHandle edge : edges)
            oriented.push_back({edge, BRepOrientation::Forward});
        return makeWire(oriented, closed);
    }

    [[nodiscard]] BRepFaceHandle makePlanarFace(
        BRepWireHandle outerWire,
        const Vector3& planeOrigin,
        const Vector3& planeNormal,
        const Vector3& planeXDir,
        const std::vector<BRepWireHandle>& holes = {},
        double tolerance = DefaultBRepTolerance.linear)
    {
        const auto* outer = model_.topology().wire(outerWire);
        if (!outer || !outer->closed || outer->edges.size() < 3)
            return {};
        if (!(tolerance > 0.0) || planeNormal.isZero() || planeXDir.isZero())
            return {};

        for (BRepWireHandle hole : holes)
        {
            const auto* holeRecord = model_.topology().wire(hole);
            if (!holeRecord || !holeRecord->closed || holeRecord->edges.size() < 3)
                return {};
            if (hole == outerWire)
                return {};
            if (holeRecord->ownerFace.valid())
                return {};
        }

        BRepSurfaceGeometry surface{};
        surface.type = BRepSurfaceType::Plane;
        surface.uRange = {-1.0e6, 1.0e6};
        surface.vRange = {-1.0e6, 1.0e6};
        surface.plane.location = planeOrigin;
        surface.plane.normal = planeNormal.normalized();
        surface.plane.xDir = planeXDir.normalized();

        if (!surface.isValid())
            return {};

        const auto checkpoint = model_.checkpoint();
        BRepFace face{};
        face.surface = model_.geometry().addSurface(std::move(surface));
        face.orientation = BRepOrientation::Forward;
        face.tolerance = tolerance;
        face.free = false;

        const BRepFaceHandle handle = model_.topology().addFace(std::move(face));
        if (!BRepTopologyEditor::setOuterWire(
                model_.topology(), handle,
                BRepOrientedWire{outerWire, BRepOrientation::Forward}))
        {
            model_.rollback(checkpoint);
            return {};
        }

        for (BRepWireHandle hole : holes)
        {
            if (!BRepTopologyEditor::addInnerWire(
                    model_.topology(), handle,
                    BRepOrientedWire{hole, BRepOrientation::Forward}))
            {
                model_.rollback(checkpoint);
                return {};
            }
        }

        return handle;
    }

    [[nodiscard]] BRepShellHandle makeShell(
        const std::vector<BRepOrientedFace>& faces,
        bool closed)
    {
        if (faces.empty())
            return {};

        if (!facesCanFormShell(faces))
            return {};

        const auto checkpoint = model_.checkpoint();
        BRepShell shell{};
        shell.closed = false;
        shell.free = false;
        const BRepShellHandle handle = model_.topology().addShell(std::move(shell));

        for (const BRepOrientedFace& item : faces)
        {
            if (!BRepTopologyEditor::addFaceToShell(model_.topology(), handle, item))
            {
                model_.rollback(checkpoint);
                return {};
            }
        }

        if (closed && !BRepTopologyEditor::markShellClosed(model_.topology(), handle, true))
        {
            model_.rollback(checkpoint);
            return {};
        }

        return handle;
    }

    [[nodiscard]] BRepSolidHandle makeSolid(
        const std::vector<BRepShellHandle>& shells,
        bool addAsRoot = true)
    {
        if (shells.empty())
            return {};

        std::vector<BRepShellHandle> uniqueShells;
        uniqueShells.reserve(shells.size());
        for (BRepShellHandle shell : shells)
        {
            if (!model_.topology().shell(shell))
                return {};
            if (std::find(uniqueShells.begin(), uniqueShells.end(), shell) != uniqueShells.end())
                return {};
            if (model_.topology().shell(shell)->ownerSolid.valid())
                return {};
            uniqueShells.push_back(shell);
        }

        const auto checkpoint = model_.checkpoint();
        BRepSolid solid{};
        solid.free = false;
        const BRepSolidHandle handle = model_.topology().addSolid(std::move(solid));

        for (BRepShellHandle shell : uniqueShells)
        {
            if (!BRepTopologyEditor::addShellToSolid(model_.topology(), handle, shell))
            {
                model_.rollback(checkpoint);
                return {};
            }
        }

        if (addAsRoot)
            model_.addRootSolid(handle);

        return handle;
    }

private:
    [[nodiscard]] static BRepVertexHandle orientedStart(
        const BRepTopologyStore& topology,
        const BRepOrientedEdge& item) noexcept
    {
        const auto* edge = topology.edge(item.edge);
        if (!edge)
            return {};
        return isForward(item.orientation) ? edge->start : edge->end;
    }

    [[nodiscard]] static BRepVertexHandle orientedEnd(
        const BRepTopologyStore& topology,
        const BRepOrientedEdge& item) noexcept
    {
        const auto* edge = topology.edge(item.edge);
        if (!edge)
            return {};
        return isForward(item.orientation) ? edge->end : edge->start;
    }

    [[nodiscard]] bool edgesCanFormWire(
        const std::vector<BRepOrientedEdge>& edges,
        bool closed) const noexcept
    {
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto& item = edges[i];
            if (!item.valid() || !model_.topology().edge(item.edge))
                return false;
            if (std::find(edges.begin(), edges.begin() + static_cast<std::ptrdiff_t>(i), item) !=
                edges.begin() + static_cast<std::ptrdiff_t>(i))
                return false;
            if (i > 0 && orientedEnd(model_.topology(), edges[i - 1]) !=
                              orientedStart(model_.topology(), edges[i]))
                return false;
        }

        if (!closed)
            return true;

        const auto first = orientedStart(model_.topology(), edges.front());
        const auto last = orientedEnd(model_.topology(), edges.back());
        return first.valid() && last.valid() && first == last;
    }

    [[nodiscard]] bool facesCanFormShell(
        const std::vector<BRepOrientedFace>& faces) const noexcept
    {
        for (std::size_t i = 0; i < faces.size(); ++i)
        {
            const auto& item = faces[i];
            const auto* face = model_.topology().face(item.face);
            if (!item.valid() || !face || !face->outer.valid())
                return false;
            if (face->ownerShell.valid())
                return false;
            if (std::find(faces.begin(), faces.begin() + static_cast<std::ptrdiff_t>(i), item) !=
                faces.begin() + static_cast<std::ptrdiff_t>(i))
                return false;
        }
        return true;
    }

    BRepModel& model_;
};

}