#pragma once

// MirEngine/BRep/BRepMakeRectangleProfile.hpp
//
// Строит замкнутый прямоугольный wire в плоскости XY (или заданной плоскости)
// как BRep-профиль для последующего Extrude.
//
// Это мост к эталонному сценарию:
//   width/height → profile wire → extrude → solid

#include "BRepBuilderAPI.hpp"
#include "../Validator/BRepValidator.hpp"

#include <array>
#include <cmath>

namespace mir
{

struct BRepRectangleProfileResult
{
    bool success{false};
    BRepWireHandle wire{};
    std::array<BRepVertexHandle, 4> vertices{};
    std::array<BRepEdgeHandle, 4> edges{};
    BRepValidationReport report{};
};

class BRepMakeRectangleProfile
{
public:
    // Прямоугольник в плоскости, заданной origin + normal + xDir.
    // width вдоль xDir, height вдоль yDir = normal × xDir.
    [[nodiscard]] static BRepRectangleProfileResult build(
        BRepModel& model,
        Scalar width,
        Scalar height,
        const Vector3& origin = Vector3::zero(),
        const Vector3& normal = Vector3::unitZ(),
        const Vector3& xDirection = Vector3::unitX(),
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        BRepRectangleProfileResult result{};

        if (!(width > 0.0) || !(height > 0.0) ||
            !std::isfinite(width) || !std::isfinite(height) ||
            !origin.isFinite() || !normal.isFinite() || !xDirection.isFinite())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Wire,
                InvalidBRepIndex,
                "Rectangle profile requires finite positive width/height and finite frame");
            return result;
        }

        const Vector3 n = normal.normalized();
        Vector3 xDir = xDirection.normalized();
        if (n.isZero() || xDir.isZero())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Wire,
                InvalidBRepIndex,
                "Rectangle profile frame is degenerate");
            return result;
        }

        // Ортогонализуем xDir относительно normal.
        xDir = (xDir - n * Vector3::dot(xDir, n));
        if (xDir.isZero())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Wire,
                InvalidBRepIndex,
                "Rectangle xDirection is parallel to normal");
            return result;
        }
        xDir = xDir.normalized();
        const Vector3 yDir = Vector3::cross(n, xDir).normalized();

        const Vector3 p0 = origin;
        const Vector3 p1 = origin + xDir * width;
        const Vector3 p2 = origin + xDir * width + yDir * height;
        const Vector3 p3 = origin + yDir * height;

        BRepBuilderAPI builder(model);

        result.vertices[0] = builder.makeVertex(p0, tolerance.linear);
        result.vertices[1] = builder.makeVertex(p1, tolerance.linear);
        result.vertices[2] = builder.makeVertex(p2, tolerance.linear);
        result.vertices[3] = builder.makeVertex(p3, tolerance.linear);

        for (std::size_t i = 0; i < 4; ++i)
        {
            if (!result.vertices[i].valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Vertex,
                    static_cast<BRepIndex>(i),
                    "Rectangle profile failed to create vertex");
                return result;
            }
        }

        result.edges[0] = builder.makeEdgeLine(
            result.vertices[0], result.vertices[1], tolerance.linear);
        result.edges[1] = builder.makeEdgeLine(
            result.vertices[1], result.vertices[2], tolerance.linear);
        result.edges[2] = builder.makeEdgeLine(
            result.vertices[2], result.vertices[3], tolerance.linear);
        result.edges[3] = builder.makeEdgeLine(
            result.vertices[3], result.vertices[0], tolerance.linear);

        for (std::size_t i = 0; i < 4; ++i)
        {
            if (!result.edges[i].valid())
            {
                result.report.add(
                    BRepValidationSeverity::Error,
                    BRepShapeType::Edge,
                    static_cast<BRepIndex>(i),
                    "Rectangle profile failed to create edge");
                return result;
            }
        }

        result.wire = builder.makeWireFromEdges(
            {result.edges[0], result.edges[1], result.edges[2], result.edges[3]},
            true);

        if (!result.wire.valid())
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Wire,
                InvalidBRepIndex,
                "Rectangle profile failed to create closed wire");
            return result;
        }

        // Лёгкая проверка связности через validator на временной модели не нужна:
        // wire ещё не в solid. Проверяем локально.
        const BRepWire* wire = model.topology().wire(result.wire);
        if (!wire || !wire->closed || wire->edges.size() != 4)
        {
            result.report.add(
                BRepValidationSeverity::Error,
                BRepShapeType::Wire,
                result.wire.index,
                "Rectangle profile wire is not a closed 4-edge loop");
            return result;
        }

        result.success = true;
        return result;
    }

    // Удобный helper: XY-плоскость, origin в углу.
    [[nodiscard]] static BRepRectangleProfileResult buildXY(
        BRepModel& model,
        Scalar width,
        Scalar height,
        const Vector3& origin = Vector3::zero(),
        BRepTolerance tolerance = DefaultBRepTolerance)
    {
        return build(
            model,
            width,
            height,
            origin,
            Vector3::unitZ(),
            Vector3::unitX(),
            tolerance);
    }
};

} // namespace mir
