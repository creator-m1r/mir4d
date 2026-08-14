#pragma once

// MirEngine/BRep/BRepPrimAPI_MakeBox.hpp
//
// Построение прямоугольного параллелепипеда как полноценного BRep solid:
// 8 vertices, 12 edges, 6 planar faces, 1 closed shell, 1 solid.

#include "BRepBuilderAPI.hpp"
#include "BRepValidator.hpp"

#include <array>
#include <cmath>

namespace mir
{

struct BRepMakeBoxResult
{
    bool success{false};
    BRepSolidHandle solid{};
    BRepValidationReport report{};
};

class BRepPrimAPI_MakeBox
{
public:
    // Box от origin с размерами dx, dy, dz (все > 0).
    [[nodiscard]] static BRepMakeBoxResult build(
        BRepModel& model,
        Scalar dx,
        Scalar dy,
        Scalar dz,
        const Vector3& origin = Vector3::zero(),
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        BRepMakeBoxResult result{};

        if (!(dx > 0.0) || !(dy > 0.0) || !(dz > 0.0) ||
            !std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz) ||
            !origin.isFinite())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "MakeBox requires finite positive dimensions and finite origin");
            return result;
        }

        BRepBuilderAPI builder(model);

        // Vertex numbering:
        //   4------5
        //  /|     /|
        // 7------6 |
        // | 0----|-1
        // |/     |/
        // 3------2
        //
        // z up, y depth, x width

        const Vector3 p0 = origin;
        const Vector3 p1 = origin + Vector3{dx, 0.0, 0.0};
        const Vector3 p2 = origin + Vector3{dx, dy, 0.0};
        const Vector3 p3 = origin + Vector3{0.0, dy, 0.0};
        const Vector3 p4 = origin + Vector3{0.0, 0.0, dz};
        const Vector3 p5 = origin + Vector3{dx, 0.0, dz};
        const Vector3 p6 = origin + Vector3{dx, dy, dz};
        const Vector3 p7 = origin + Vector3{0.0, dy, dz};

        const std::array<Vector3, 8> points{p0, p1, p2, p3, p4, p5, p6, p7};
        std::array<BRepVertexHandle, 8> vertices{};

        for (std::size_t i = 0; i < points.size(); ++i)
        {
            vertices[i] = builder.makeVertex(points[i], tolerance.linear);
            if (!vertices[i].valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Vertex,
                    static_cast<BRepIndex>(i),
                    "MakeBox failed to create vertex");
                return result;
            }
        }

        auto edge = [&](std::size_t a, std::size_t b) -> BRepEdgeHandle
        {
            return builder.makeEdgeLine(vertices[a], vertices[b], tolerance.linear);
        };

        // Bottom
        const BRepEdgeHandle e01 = edge(0, 1);
        const BRepEdgeHandle e12 = edge(1, 2);
        const BRepEdgeHandle e23 = edge(2, 3);
        const BRepEdgeHandle e30 = edge(3, 0);
        // Top
        const BRepEdgeHandle e45 = edge(4, 5);
        const BRepEdgeHandle e56 = edge(5, 6);
        const BRepEdgeHandle e67 = edge(6, 7);
        const BRepEdgeHandle e74 = edge(7, 4);
        // Vertical
        const BRepEdgeHandle e04 = edge(0, 4);
        const BRepEdgeHandle e15 = edge(1, 5);
        const BRepEdgeHandle e26 = edge(2, 6);
        const BRepEdgeHandle e37 = edge(3, 7);

        const std::array<BRepEdgeHandle, 12> allEdges{
            e01, e12, e23, e30, e45, e56, e67, e74, e04, e15, e26, e37};

        for (std::size_t i = 0; i < allEdges.size(); ++i)
        {
            if (!allEdges[i].valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Edge,
                    static_cast<BRepIndex>(i),
                    "MakeBox failed to create edge");
                return result;
            }
        }

        auto oriented = [](BRepEdgeHandle h, BRepOrientation o) -> BRepOrientedEdge
        {
            return {h, o};
        };

        // Wires are oriented CCW when viewed against face normal (outward).

        // Bottom face normal -Z
        const BRepWireHandle bottomWire = builder.makeWire(
            {
                oriented(e01, BRepOrientation::Reversed),
                oriented(e30, BRepOrientation::Reversed),
                oriented(e23, BRepOrientation::Reversed),
                oriented(e12, BRepOrientation::Reversed)
            },
            true);

        // Top face normal +Z
        const BRepWireHandle topWire = builder.makeWire(
            {
                oriented(e45, BRepOrientation::Forward),
                oriented(e56, BRepOrientation::Forward),
                oriented(e67, BRepOrientation::Forward),
                oriented(e74, BRepOrientation::Forward)
            },
            true);

        // Front face (y=0) normal -Y
        const BRepWireHandle frontWire = builder.makeWire(
            {
                oriented(e01, BRepOrientation::Forward),
                oriented(e15, BRepOrientation::Forward),
                oriented(e45, BRepOrientation::Reversed),
                oriented(e04, BRepOrientation::Reversed)
            },
            true);

        // Back face (y=dy) normal +Y
        const BRepWireHandle backWire = builder.makeWire(
            {
                oriented(e23, BRepOrientation::Forward),
                oriented(e37, BRepOrientation::Forward),
                oriented(e67, BRepOrientation::Reversed),
                oriented(e26, BRepOrientation::Reversed)
            },
            true);

        // Left face (x=0) normal -X
        const BRepWireHandle leftWire = builder.makeWire(
            {
                oriented(e30, BRepOrientation::Forward),
                oriented(e04, BRepOrientation::Forward),
                oriented(e74, BRepOrientation::Reversed),
                oriented(e37, BRepOrientation::Reversed)
            },
            true);

        // Right face (x=dx) normal +X
        const BRepWireHandle rightWire = builder.makeWire(
            {
                oriented(e12, BRepOrientation::Forward),
                oriented(e26, BRepOrientation::Forward),
                oriented(e56, BRepOrientation::Reversed),
                oriented(e15, BRepOrientation::Reversed)
            },
            true);

        const std::array<BRepWireHandle, 6> wires{
            bottomWire, topWire, frontWire, backWire, leftWire, rightWire};

        for (std::size_t i = 0; i < wires.size(); ++i)
        {
            if (!wires[i].valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Wire,
                    static_cast<BRepIndex>(i),
                    "MakeBox failed to create face wire");
                return result;
            }
        }

        const Vector3 center = origin + Vector3{dx * 0.5, dy * 0.5, dz * 0.5};

        const BRepFaceHandle bottomFace = builder.makePlanarFace(
            bottomWire,
            origin,
            Vector3{0.0, 0.0, -1.0},
            Vector3{1.0, 0.0, 0.0},
            {},
            tolerance.linear);

        const BRepFaceHandle topFace = builder.makePlanarFace(
            topWire,
            origin + Vector3{0.0, 0.0, dz},
            Vector3{0.0, 0.0, 1.0},
            Vector3{1.0, 0.0, 0.0},
            {},
            tolerance.linear);

        const BRepFaceHandle frontFace = builder.makePlanarFace(
            frontWire,
            origin,
            Vector3{0.0, -1.0, 0.0},
            Vector3{1.0, 0.0, 0.0},
            {},
            tolerance.linear);

        const BRepFaceHandle backFace = builder.makePlanarFace(
            backWire,
            origin + Vector3{0.0, dy, 0.0},
            Vector3{0.0, 1.0, 0.0},
            Vector3{1.0, 0.0, 0.0},
            {},
            tolerance.linear);

        const BRepFaceHandle leftFace = builder.makePlanarFace(
            leftWire,
            origin,
            Vector3{-1.0, 0.0, 0.0},
            Vector3{0.0, 1.0, 0.0},
            {},
            tolerance.linear);

        const BRepFaceHandle rightFace = builder.makePlanarFace(
            rightWire,
            origin + Vector3{dx, 0.0, 0.0},
            Vector3{1.0, 0.0, 0.0},
            Vector3{0.0, 1.0, 0.0},
            {},
            tolerance.linear);

        const std::array<BRepFaceHandle, 6> faces{
            bottomFace, topFace, frontFace, backFace, leftFace, rightFace};

        for (std::size_t i = 0; i < faces.size(); ++i)
        {
            if (!faces[i].valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Face,
                    static_cast<BRepIndex>(i),
                    "MakeBox failed to create face");
                return result;
            }
        }

        std::vector<BRepOrientedFace> orientedFaces;
        orientedFaces.reserve(faces.size());
        for (BRepFaceHandle face : faces)
            orientedFaces.push_back({face, BRepOrientation::Forward});

        const BRepShellHandle shell = builder.makeShell(orientedFaces, true);
        if (!shell.valid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Shell,
                InvalidBRepIndex,
                "MakeBox failed to create shell");
            return result;
        }

        const BRepSolidHandle solid = builder.makeSolid({shell}, true);
        if (!solid.valid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "MakeBox failed to create solid");
            return result;
        }

        // Suppress unused-variable warning if center is not used later.
        (void)center;

        const BRepValidator validator(tolerance);
        result.report = validator.validate(model);
        result.solid = solid;
        result.success = result.report.ok() && solid.valid();
        return result;
    }
};

} // namespace mir
