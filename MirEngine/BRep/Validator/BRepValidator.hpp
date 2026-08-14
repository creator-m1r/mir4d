#pragma once

// MirEngine/BRep/Validator/BRepValidator.hpp
// Structural and geometric validation for the BRep model.

#include "MirEngine/BRep/Core/BRepModel.hpp"
#include "MirEngine/BRep/Topology/BRepTypes.hpp"

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace mir
{

enum class BRepValidationSeverity : std::uint8_t
{
    Info = 0,
    Warning,
    Error
};

struct BRepValidationIssue
{
    BRepValidationSeverity severity{BRepValidationSeverity::Error};
    BRepShapeType shapeType{BRepShapeType::Unknown};
    BRepIndex index{InvalidBRepIndex};
    std::string message;
};

struct BRepValidationReport
{
    std::vector<BRepValidationIssue> issues;

    [[nodiscard]] bool ok() const noexcept
    {
        for (const BRepValidationIssue& issue : issues)
            if (issue.severity == BRepValidationSeverity::Error) return false;
        return true;
    }

    void add(BRepValidationSeverity severity, BRepShapeType shapeType,
             BRepIndex index, std::string message)
    {
        issues.push_back({severity, shapeType, index, std::move(message)});
    }
};

class BRepValidator
{
public:
    explicit BRepValidator(BRepTolerance tolerance = DefaultBRepTolerance) noexcept
        : tolerance_(tolerance) {}

    [[nodiscard]] BRepValidationReport validate(const BRepModel& model) const
    {
        BRepValidationReport report;
        if (!tolerance_.isValid())
        {
            report.add(BRepValidationSeverity::Error, BRepShapeType::Unknown,
                      InvalidBRepIndex, "Invalid BRep tolerance settings");
            return report;
        }

        validateGeometry(model, report);
        validateVertices(model, report);
        validateEdges(model, report);
        validateWires(model, report);
        validateFaces(model, report);
        validateShells(model, report);
        validateSolids(model, report);
        validateRoots(model, report);
        return report;
    }

private:
    void validateGeometry(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& geometry = model.geometry();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(geometry.pointCount()); ++i)
            if (!geometry.point(BRepPointHandle{i}) || !geometry.point(BRepPointHandle{i})->isFinite())
                report.add(BRepValidationSeverity::Error, BRepShapeType::Vertex, i, "Point geometry is missing or non-finite");
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(geometry.curveCount()); ++i)
            if (!geometry.curve(BRepCurveHandle{i}) || !geometry.curve(BRepCurveHandle{i})->isValid())
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Curve geometry is missing or invalid");
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(geometry.surfaceCount()); ++i)
            if (!geometry.surface(BRepSurfaceHandle{i}) || !geometry.surface(BRepSurfaceHandle{i})->isValid())
                report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Surface geometry is missing or invalid");
    }

    void validateVertices(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& topology = model.topology();
        const auto& geometry = model.geometry();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(topology.vertexCount()); ++i)
        {
            const auto* vertex = topology.vertex(BRepVertexHandle{i});
            if (!vertex)
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Vertex, i, "Vertex slot is empty");
                continue;
            }
            if (!vertex->isValid())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Vertex, i, "Vertex record is invalid");
                continue;
            }
            if (!geometry.point(vertex->point))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Vertex, i, "Vertex references missing point geometry");
        }
    }

    void validateEdges(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& topology = model.topology();
        const auto& geometry = model.geometry();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(topology.edgeCount()); ++i)
        {
            const auto* edge = topology.edge(BRepEdgeHandle{i});
            if (!edge)
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Edge slot is empty");
                continue;
            }
            if (!edge->isValid())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Edge record is invalid");
                continue;
            }
            if (edge->degenerated) continue;

            const auto* curve = geometry.curve(edge->curve);
            if (!curve)
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Edge references missing curve geometry");
                continue;
            }
            if (!topology.vertex(edge->start) || !topology.vertex(edge->end))
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Edge references missing start/end vertex");
                continue;
            }

            const Vector3 p0 = curveEndpoint(model, *edge, true);
            const Vector3 p1 = curveEndpoint(model, *edge, false);
            const Vector3 v0 = vertexPoint(model, edge->start);
            const Vector3 v1 = vertexPoint(model, edge->end);
            const double maxTol = std::max(edge->tolerance, tolerance_.linear) * 10.0;
            if ((p0 - v0).length() > maxTol)
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Edge start vertex does not match curve start within tolerance");
            if ((p1 - v1).length() > maxTol)
                report.add(BRepValidationSeverity::Error, BRepShapeType::Edge, i, "Edge end vertex does not match curve end within tolerance");
        }
    }

    void validateWires(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& topology = model.topology();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(topology.wireCount()); ++i)
        {
            const auto* wire = topology.wire(BRepWireHandle{i});
            if (!wire || !wire->isValid())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Wire, i, "Wire record is invalid");
                continue;
            }
            if (wire->edges.empty())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Wire, i, "Wire contains no edges");
                continue;
            }
            for (const auto& oriented : wire->edges)
                if (!topology.edge(oriented.edge))
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Wire, i, "Wire references missing edge");
            if (!wireConnectivityOk(model, *wire))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Wire, i, "Wire edges are not sequentially connected");
            if (wire->closed && !wireClosedOk(model, *wire))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Wire, i, "Wire is marked closed but endpoints do not match");
            if (wire->ownerFace.valid() && !topology.face(wire->ownerFace))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Wire, i, "Wire owner face is dangling");
        }
    }

    void validateFaces(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& topology = model.topology();
        const auto& geometry = model.geometry();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(topology.faceCount()); ++i)
        {
            const auto* face = topology.face(BRepFaceHandle{i});
            if (!face || !face->isValid())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face record is invalid");
                continue;
            }
            if (!geometry.surface(face->surface))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face references missing surface geometry");

            const auto* outer = topology.wire(face->outer.wire);
            if (!outer)
                report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face outer wire is missing");
            else if (outer->ownerFace.valid() && outer->ownerFace != face->self)
                report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face outer wire belongs to another face");

            for (const auto& inner : face->inners)
            {
                const auto* wire = topology.wire(inner.wire);
                if (!wire)
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face inner wire is missing");
                else if (wire->ownerFace.valid() && wire->ownerFace != face->self)
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face inner wire belongs to another face");
            }

            if (face->ownerShell.valid() && !topology.shell(face->ownerShell))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Face, i, "Face owner shell is dangling");
        }
    }

    void validateShells(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& topology = model.topology();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(topology.shellCount()); ++i)
        {
            const auto* shell = topology.shell(BRepShellHandle{i});
            if (!shell || !shell->isValid())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Shell, i, "Shell record is invalid");
                continue;
            }
            if (shell->faces.empty())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Shell, i, "Shell contains no faces");
                continue;
            }
            for (const auto& oriented : shell->faces)
            {
                const auto* face = topology.face(oriented.face);
                if (!face)
                {
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Shell, i, "Shell references missing face");
                    continue;
                }
                if (face->ownerShell.valid() && face->ownerShell != shell->self)
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Shell, i, "Shell contains a face owned by another shell");
            }
            if (shell->ownerSolid.valid() && !topology.solid(shell->ownerSolid))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Shell, i, "Shell owner solid is dangling");
        }
    }

    void validateSolids(const BRepModel& model, BRepValidationReport& report) const
    {
        const auto& topology = model.topology();
        for (BRepIndex i = 0; i < static_cast<BRepIndex>(topology.solidCount()); ++i)
        {
            const auto* solid = topology.solid(BRepSolidHandle{i});
            if (!solid || !solid->isValid())
            {
                report.add(BRepValidationSeverity::Error, BRepShapeType::Solid, i, "Solid record is invalid");
                continue;
            }
            for (BRepShellHandle shellHandle : solid->shells)
            {
                const auto* shell = topology.shell(shellHandle);
                if (!shell)
                {
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Solid, i, "Solid references missing shell");
                    continue;
                }
                if (shell->ownerSolid.valid() && shell->ownerSolid != solid->self)
                    report.add(BRepValidationSeverity::Error, BRepShapeType::Solid, i, "Solid contains a shell owned by another solid");
            }
        }
    }

    void validateRoots(const BRepModel& model, BRepValidationReport& report) const
    {
        if (model.rootSolids().empty())
            report.add(BRepValidationSeverity::Warning, BRepShapeType::Solid, InvalidBRepIndex, "BRep model has no root solids");
        for (BRepSolidHandle solidHandle : model.rootSolids())
            if (!model.topology().solid(solidHandle))
                report.add(BRepValidationSeverity::Error, BRepShapeType::Solid, solidHandle.index, "Root solid handle is dangling");
    }

    [[nodiscard]] static Vector3 vertexPoint(const BRepModel& model, BRepVertexHandle vertexHandle) noexcept
    {
        const auto* vertex = model.topology().vertex(vertexHandle);
        if (!vertex) return Vector3::zero();
        const auto* point = model.geometry().point(vertex->point);
        return point ? point->point : Vector3::zero();
    }

    [[nodiscard]] static Vector3 curveEndpoint(const BRepModel& model, const BRepEdge& edge, bool start) noexcept
    {
        const auto* curve = model.geometry().curve(edge.curve);
        if (!curve) return Vector3::zero();
        return curve->valueAt(start ? edge.range.first : edge.range.last);
    }

    [[nodiscard]] static BRepVertexHandle orientedStart(const BRepModel& model, const BRepOrientedEdge& oriented) noexcept
    {
        const auto* edge = model.topology().edge(oriented.edge);
        if (!edge) return {};
        return isForward(oriented.orientation) ? edge->start : edge->end;
    }

    [[nodiscard]] static BRepVertexHandle orientedEnd(const BRepModel& model, const BRepOrientedEdge& oriented) noexcept
    {
        const auto* edge = model.topology().edge(oriented.edge);
        if (!edge) return {};
        return isForward(oriented.orientation) ? edge->end : edge->start;
    }

    [[nodiscard]] static bool wireConnectivityOk(const BRepModel& model, const BRepWire& wire) noexcept
    {
        if (wire.edges.empty()) return false;
        for (std::size_t i = 1; i < wire.edges.size(); ++i)
            if (!orientedEnd(model, wire.edges[i - 1]).valid() ||
                orientedEnd(model, wire.edges[i - 1]) != orientedStart(model, wire.edges[i])) return false;
        return true;
    }

    [[nodiscard]] static bool wireClosedOk(const BRepModel& model, const BRepWire& wire) noexcept
    {
        if (wire.edges.empty()) return false;
        const auto first = orientedStart(model, wire.edges.front());
        const auto last = orientedEnd(model, wire.edges.back());
        return first.valid() && last.valid() && first == last;
    }

    BRepTolerance tolerance_;
};

} // namespace mir
