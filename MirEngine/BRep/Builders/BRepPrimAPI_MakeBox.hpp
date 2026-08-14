#pragma once

// MirEngine/BRep/BRepPrimAPI_MakeBox.hpp
//
// Построение прямоугольного параллелепипеда как полноценного BRep solid:
// 8 vertices, 12 edges, 6 planar faces, 1 closed shell, 1 solid.
//
// buildOriented — тот же box в произвольном ортонормированном базисе
// (повёрнутый box). Фундамент обобщения булева kernel на произвольные
// грани (P2.4): оси-параллельный box — частный случай единичного базиса.

#include "BRepBuilderAPI.hpp"
#include "../Validator/BRepValidator.hpp"

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
    // Box от origin с размерами dx, dy, dz (все > 0) в осях X/Y/Z.
    [[nodiscard]] static BRepMakeBoxResult build(
        BRepModel& model,
        Scalar dx,
        Scalar dy,
        Scalar dz,
        const Vector3& origin = Vector3::zero(),
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        return buildOriented(
            model, dx, dy, dz, origin,
            Vector3::unitX(), Vector3::unitY(),
            tolerance);
    }

    // Box в произвольном ортонормированном базисе: рёбра длиной dx вдоль
    // axisX, dy вдоль axisY, dz вдоль axisZ = cross(axisX, axisY).
    // axisX и axisY должны быть единичными и ортогональными.
    [[nodiscard]] static BRepMakeBoxResult buildOriented(
        BRepModel& model,
        Scalar dx,
        Scalar dy,
        Scalar dz,
        const Vector3& origin,
        const Vector3& axisX,
        const Vector3& axisY,
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        BRepMakeBoxResult result{};

        if (!(dx > 0.0) || !(dy > 0.0) || !(dz > 0.0) ||
            !std::isfinite(dx) || !std::isfinite(dy) || !std::isfinite(dz) ||
            !origin.isFinite() || !axisX.isFinite() || !axisY.isFinite())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "MakeBox requires finite positive dimensions, finite origin and finite basis");
            return result;
        }

        const Scalar axisXEpsilon = std::abs(axisX.length() - 1.0);
        const Scalar axisYEpsilon = std::abs(axisY.length() - 1.0);
        const Scalar axisOrthogonality = std::abs(Vector3::dot(axisX, axisY));

        constexpr Scalar basisTolerance = Scalar(1e-6);

        if (axisXEpsilon > basisTolerance ||
            axisYEpsilon > basisTolerance ||
            axisOrthogonality > basisTolerance)
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Solid,
                InvalidBRepIndex,
                "MakeBoxOriented requires an orthonormal basis (unit, orthogonal axes)");
            return result;
        }

        const Vector3 axisZ = Vector3::cross(axisX, axisY);

        BRepBuilderAPI builder(model);

        // Vertex numbering:
        //   4------5
        //  /|     /|
        // 7------6 |
        // | 0----|-1
        // |/     |/
        // 3------2
        //
        // axisZ up, axisY depth, axisX width

        const Vector3 p0 = origin;
        const Vector3 p1 = origin + axisX * dx;
        const Vector3 p2 = origin + axisX * dx + axisY * dy;
        const Vector3 p3 = origin + axisY * dy;
        const Vector3 p4 = origin + axisZ * dz;
        const Vector3 p5 = origin + axisX * dx + axisZ * dz;
        const Vector3 p6 = origin + axisX * dx + axisY * dy + axisZ * dz;
        const Vector3 p7 = origin + axisY * dy + axisZ * dz;

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

        // Bottom face normal -axisZ
        const BRepWireHandle bottomWire = builder.makeWire(
            {
                oriented(e01, BRepOrientation::Reversed),
                oriented(e30, BRepOrientation::Reversed),
                oriented(e23, BRepOrientation::Reversed),
                oriented(e12, BRepOrientation::Reversed)
            },
            true);

        // Top face normal +axisZ
        const BRepWireHandle topWire = builder.makeWire(
            {
                oriented(e45, BRepOrientation::Forward),
                oriented(e56, BRepOrientation::Forward),
                oriented(e67, BRepOrientation::Forward),
                oriented(e74, BRepOrientation::Forward)
            },
            true);

        // Front face (v=0) normal -axisY
        const BRepWireHandle frontWire = builder.makeWire(
            {
                oriented(e01, BRepOrientation::Forward),
                oriented(e15, BRepOrientation::Forward),
                oriented(e45, BRepOrientation::Reversed),
                oriented(e04, BRepOrientation::Reversed)
            },
            true);

        // Back face (v=dy) normal +axisY
        const BRepWireHandle backWire = builder.makeWire(
            {
                oriented(e23, BRepOrientation::Forward),
                oriented(e37, BRepOrientation::Forward),
                oriented(e67, BRepOrientation::Reversed),
                oriented(e26, BRepOrientation::Reversed)
            },
            true);

        // Left face (u=0) normal -axisX
        const BRepWireHandle leftWire = builder.makeWire(
            {
                oriented(e30, BRepOrientation::Forward),
                oriented(e04, BRepOrientation::Forward),
                oriented(e74, BRepOrientation::Reversed),
                oriented(e37, BRepOrientation::Reversed)
            },
            true);

        // Right face (u=dx) normal +axisX
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

        const Vector3 center = origin +
            axisX * (dx * 0.5) + axisY * (dy * 0.5) + axisZ * (dz * 0.5);

        const BRepFaceHandle bottomFace = builder.makePlanarFace(
            bottomWire,
            origin,
            -axisZ,
            axisX,
            {},
            tolerance.linear);

        const BRepFaceHandle topFace = builder.makePlanarFace(
            topWire,
            origin + axisZ * dz,
            axisZ,
            axisX,
            {},
            tolerance.linear);

        const BRepFaceHandle frontFace = builder.makePlanarFace(
            frontWire,
            origin,
            -axisY,
            axisX,
            {},
            tolerance.linear);

        const BRepFaceHandle backFace = builder.makePlanarFace(
            backWire,
            origin + axisY * dy,
            axisY,
            axisX,
            {},
            tolerance.linear);

        const BRepFaceHandle leftFace = builder.makePlanarFace(
            leftWire,
            origin,
            -axisX,
            axisY,
            {},
            tolerance.linear);

        const BRepFaceHandle rightFace = builder.makePlanarFace(
            rightWire,
            origin + axisX * dx,
            axisX,
            axisY,
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

        (void)center;

        const BRepValidator validator(tolerance);
        result.report = validator.validate(model);
        result.solid = solid;
        result.success = result.report.ok() && solid.valid();
        return result;
    }
};

} // namespace mir
