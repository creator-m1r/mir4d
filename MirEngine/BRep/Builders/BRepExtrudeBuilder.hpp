#pragma once

#include "BRepBuilderAPI.hpp"
#include "../Validator/BRepValidator.hpp"

#include <cmath>
#include <vector>

namespace mir
{

struct BRepExtrudeResult
{
    bool success{false};
    BRepSolidHandle solid{};
    BRepValidationReport report{};
};

class BRepExtrudeBuilder
{
public:
    [[nodiscard]] static BRepExtrudeResult extrudeWire(
        BRepModel& model,
        BRepWireHandle profileWire,
        const Vector3& direction,
        Scalar distance,
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        return extrudeWireWithHoles(
            model,
            profileWire,
            {},
            direction,
            distance,
            tolerance);
    }

    [[nodiscard]] static BRepExtrudeResult extrudeWireWithHoles(
        BRepModel& model,
        BRepWireHandle outerWire,
        const std::vector<BRepWireHandle>& holeWires,
        const Vector3& direction,
        Scalar distance,
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        BRepExtrudeResult result{};

        if (!(distance > 0.0) || !std::isfinite(distance) || !direction.isFinite())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "Extrude requires finite positive distance and finite direction");
            return result;
        }

        const Vector3 unitDir = direction.normalized();
        if (unitDir.isZero())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "Extrude direction is degenerate");
            return result;
        }

        std::vector<BRepWireHandle> allProfiles;
        allProfiles.reserve(1 + holeWires.size());
        allProfiles.push_back(outerWire);
        for (BRepWireHandle hole : holeWires)
            allProfiles.push_back(hole);

        struct ProfilePrism
        {
            std::vector<BRepVertexHandle> bottom;
            std::vector<BRepVertexHandle> top;
            std::vector<BRepEdgeHandle> bottomEdges;
            std::vector<BRepEdgeHandle> topEdges;
            std::vector<BRepEdgeHandle> verticalEdges;
            BRepWireHandle bottomWire{};
            BRepWireHandle topWire{};
        };

        BRepBuilderAPI builder(model);
        std::vector<ProfilePrism> prisms;
        prisms.reserve(allProfiles.size());

        for (BRepWireHandle profileHandle : allProfiles)
        {
            const BRepWire* wire = model.topology().wire(profileHandle);
            if (!wire || !wire->isValid() || !wire->closed || wire->edges.size() < 3)
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Wire,
                    profileHandle.index,
                    "Extrude profile wire must be valid, closed and have >= 3 edges");
                return result;
            }

            ProfilePrism prism{};

            for (const BRepOrientedEdge& oriented : wire->edges)
            {
                const BRepEdge* edge = model.topology().edge(oriented.edge);
                if (!edge || edge->degenerated)
                {
                    result.report.add(
                        BRepValidationSeverity::Error,
                        BRepShapeType::Edge,
                        oriented.edge.index,
                        "Extrude profile contains invalid edge");
                    return result;
                }

                const BRepVertexHandle start =
                    isForward(oriented.orientation) ? edge->start : edge->end;
                prism.bottom.push_back(start);
            }

            const std::size_t n = prism.bottom.size();
            if (n < 3)
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Wire,
                    profileHandle.index,
                    "Extrude profile has insufficient vertices");
                return result;
            }

            prism.top.reserve(n);
            for (BRepVertexHandle bottom : prism.bottom)
            {
                const Vector3 p = pointOf(model, bottom);
                const BRepVertexHandle top = builder.makeVertex(
                    p + unitDir * distance,
                    tolerance.linear);
                if (!top.valid())
                {
                    result.report.add(
                        BRepValidationSeverity::Error,
                        BRepShapeType::Vertex,
                        InvalidBRepIndex,
                        "Extrude failed to create top vertex");
                    return result;
                }
                prism.top.push_back(top);
            }

            prism.bottomEdges.reserve(n);
            prism.topEdges.reserve(n);
            prism.verticalEdges.reserve(n);

            for (std::size_t i = 0; i < n; ++i)
            {
                const std::size_t j = (i + 1) % n;

                const BRepEdgeHandle bottomEdge = builder.makeEdgeLine(
                    prism.bottom[i], prism.bottom[j], tolerance.linear);
                const BRepEdgeHandle topEdge = builder.makeEdgeLine(
                    prism.top[i], prism.top[j], tolerance.linear);
                const BRepEdgeHandle verticalEdge = builder.makeEdgeLine(
                    prism.bottom[i], prism.top[i], tolerance.linear);

                if (!bottomEdge.valid() || !topEdge.valid() || !verticalEdge.valid())
                {
                    result.report.add(
                        BRepValidationSeverity::Error,
                        BRepShapeType::Edge,
                        InvalidBRepIndex,
                        "Extrude failed to create prism edges");
                    return result;
                }

                prism.bottomEdges.push_back(bottomEdge);
                prism.topEdges.push_back(topEdge);
                prism.verticalEdges.push_back(verticalEdge);
            }

            auto fwd = [](BRepEdgeHandle h) -> BRepOrientedEdge
            {
                return {h, BRepOrientation::Forward};
            };
            auto rev = [](BRepEdgeHandle h) -> BRepOrientedEdge
            {
                return {h, BRepOrientation::Reversed};
            };

            std::vector<BRepOrientedEdge> bottomLoop;
            bottomLoop.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
            {
                const std::size_t k = n - 1 - i;
                bottomLoop.push_back(rev(prism.bottomEdges[k]));
            }

            std::vector<BRepOrientedEdge> topLoop;
            topLoop.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
                topLoop.push_back(fwd(prism.topEdges[i]));

            prism.bottomWire = builder.makeWire(bottomLoop, true);
            prism.topWire = builder.makeWire(topLoop, true);
            if (!prism.bottomWire.valid() || !prism.topWire.valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Wire,
                    InvalidBRepIndex,
                    "Extrude failed to create top/bottom wires");
                return result;
            }

            prisms.push_back(std::move(prism));
        }

        const ProfilePrism& outer = prisms.front();
        const Vector3 a = pointOf(model, outer.bottom[0]);
        const Vector3 b = pointOf(model, outer.bottom[1]);
        const Vector3 c = pointOf(model, outer.bottom[2]);

        Vector3 normal = Vector3::cross(b - a, c - a);
        if (normal.isZero())
            normal = unitDir;
        else
            normal = normal.normalized();

        if (Vector3::dot(normal, unitDir) > 0.0)
            normal = Vector3{-normal.x, -normal.y, -normal.z};

        Vector3 xDir = (b - a);
        if (xDir.isZero())
            xDir = Vector3::unitX();
        else
            xDir = xDir.normalized();

        std::vector<BRepWireHandle> bottomHoles;
        std::vector<BRepWireHandle> topHoles;
        bottomHoles.reserve(prisms.size() > 0 ? prisms.size() - 1 : 0);
        topHoles.reserve(bottomHoles.capacity());
        for (std::size_t i = 1; i < prisms.size(); ++i)
        {
            bottomHoles.push_back(prisms[i].bottomWire);
            topHoles.push_back(prisms[i].topWire);
        }

        const BRepFaceHandle bottomFace = builder.makePlanarFace(
            outer.bottomWire,
            a,
            normal,
            xDir,
            bottomHoles,
            tolerance.linear);

        const BRepFaceHandle topFace = builder.makePlanarFace(
            outer.topWire,
            a + unitDir * distance,
            Vector3{-normal.x, -normal.y, -normal.z},
            xDir,
            topHoles,
            tolerance.linear);

        if (!bottomFace.valid() || !topFace.valid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Face,
                InvalidBRepIndex,
                "Extrude failed to create top/bottom faces");
            return result;
        }

        std::vector<BRepOrientedFace> shellFaces;
        shellFaces.push_back({bottomFace, BRepOrientation::Forward});
        shellFaces.push_back({topFace, BRepOrientation::Forward});

        auto fwd = [](BRepEdgeHandle h) -> BRepOrientedEdge
        {
            return {h, BRepOrientation::Forward};
        };
        auto rev = [](BRepEdgeHandle h) -> BRepOrientedEdge
        {
            return {h, BRepOrientation::Reversed};
        };

        for (const ProfilePrism& prism : prisms)
        {
            const std::size_t n = prism.bottom.size();
            for (std::size_t i = 0; i < n; ++i)
            {
                const std::size_t j = (i + 1) % n;

                const std::vector<BRepOrientedEdge> sideLoop{
                    fwd(prism.bottomEdges[i]),
                    fwd(prism.verticalEdges[j]),
                    rev(prism.topEdges[i]),
                    rev(prism.verticalEdges[i])
                };

                const BRepWireHandle sideWire = builder.makeWire(sideLoop, true);
                if (!sideWire.valid())
                {
                    result.report.add(
                        BRepValidationSeverity::Error,
                        BRepShapeType::Wire,
                        InvalidBRepIndex,
                        "Extrude failed to create side wire");
                    return result;
                }

                const Vector3 p0 = pointOf(model, prism.bottom[i]);
                const Vector3 p1 = pointOf(model, prism.bottom[j]);
                Vector3 sideX = p1 - p0;
                Vector3 sideNormal = Vector3::cross(sideX, unitDir);
                if (sideNormal.isZero())
                    sideNormal = Vector3::unitX();
                else
                    sideNormal = sideNormal.normalized();

                if (sideX.isZero())
                    sideX = Vector3::unitX();
                else
                    sideX = sideX.normalized();

                const BRepFaceHandle sideFace = builder.makePlanarFace(
                    sideWire,
                    p0,
                    sideNormal,
                    sideX,
                    {},
                    tolerance.linear);

                if (!sideFace.valid())
                {
                    result.report.add(
                        BRepValidationSeverity::Error,
                        BRepShapeType::Face,
                        InvalidBRepIndex,
                        "Extrude failed to create side face");
                    return result;
                }

                shellFaces.push_back({sideFace, BRepOrientation::Forward});
            }
        }

        const BRepShellHandle shell = builder.makeShell(shellFaces, true);
        if (!shell.valid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Shell,
                InvalidBRepIndex,
                "Extrude failed to create shell");
            return result;
        }

        const BRepSolidHandle solid = builder.makeSolid({shell}, true);
        if (!solid.valid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "Extrude failed to create solid");
            return result;
        }

        const BRepValidator validator(tolerance);
        result.report = validator.validate(model);
        result.solid = solid;
        result.success = result.report.ok() && solid.valid();
        return result;
    }

private:
    [[nodiscard]] static Vector3 pointOf(
        const BRepModel& model,
        BRepVertexHandle vertexHandle) noexcept
    {
        const BRepVertex* vertex = model.topology().vertex(vertexHandle);
        if (!vertex)
            return Vector3::zero();
        const BRepPointGeometry* point = model.geometry().point(vertex->point);
        if (!point)
            return Vector3::zero();
        return point->point;
    }
};

}
