#include "StlExporter.hpp"

#include "TessellationMeshConverter.hpp"
#include "../../Document/Document.hpp"
#include "../../Geometry/Model/ModelNode.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

namespace mir::io
{
namespace
{
struct TriangleData
{
    Point3 a;
    Point3 b;
    Point3 c;
    Vector3 normal;
};

[[nodiscard]] Vector3 triangleNormal(const Point3& a, const Point3& b, const Point3& c)
{
    const Vector3 ab{b.x - a.x, b.y - a.y, b.z - a.z};
    const Vector3 ac{c.x - a.x, c.y - a.y, c.z - a.z};
    return Vector3::cross(ab, ac).normalized();
}

[[nodiscard]] std::vector<TriangleData> collectTriangles(
    const mir4d::Document& document,
    const ExportOptions& options)
{
    std::vector<TriangleData> triangles;

    const auto include = [&](mir4d::ObjectId id)
    {
        if (!options.selectionOnly || options.selection.empty())
            return true;
        for (const mir4d::ObjectId selected : options.selection)
            if (selected == id)
                return true;
        return false;
    };

    for (const auto& node : document.scene().nodes())
    {
        if (!node || !include(node->id()) || !node->model() || !node->model()->hasMesh())
            continue;

        const TriangleMesh3& mesh = node->model()->mesh();
        const Transform& transform = node->transform();

        triangles.reserve(triangles.size() + mesh.triangles.size());
        for (const auto& triangle : mesh.triangles)
        {
            if (triangle.a >= mesh.vertices.size() ||
                triangle.b >= mesh.vertices.size() ||
                triangle.c >= mesh.vertices.size())
                continue;

            const Point3 a = transform.transformPoint(mesh.vertices[triangle.a]);
            const Point3 b = transform.transformPoint(mesh.vertices[triangle.b]);
            const Point3 c = transform.transformPoint(mesh.vertices[triangle.c]);

            triangles.push_back({a, b, c, triangleNormal(a, b, c)});
        }
    }

    return triangles;
}

void writeFloatLe(std::ostream& output, float value)
{
    const std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
    const char bytes[4]{
        static_cast<char>(bits & 0xff),
        static_cast<char>((bits >> 8) & 0xff),
        static_cast<char>((bits >> 16) & 0xff),
        static_cast<char>((bits >> 24) & 0xff)};
    output.write(bytes, 4);
}

void writeU16Le(std::ostream& output, std::uint16_t value)
{
    const char bytes[2]{static_cast<char>(value & 0xff), static_cast<char>((value >> 8) & 0xff)};
    output.write(bytes, 2);
}

void writeU32Le(std::ostream& output, std::uint32_t value)
{
    const char bytes[4]{
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff)};
    output.write(bytes, 4);
}

[[nodiscard]] bool writeBinary(
    const std::string& path,
    const std::vector<TriangleData>& triangles,
    double scale)
{
    std::ofstream output(path, std::ios::binary);
    if (!output)
        return false;

    std::array<char, 80> header{};
    const std::string text = "MIR4D STL export";
    std::copy(text.begin(), text.end(), header.begin());
    output.write(header.data(), static_cast<std::streamsize>(header.size()));
    writeU32Le(output, static_cast<std::uint32_t>(triangles.size()));

    for (const TriangleData& triangle : triangles)
    {
        const auto point = [scale](const Point3& p)
        {
            return std::array<float, 3>{
                static_cast<float>(p.x * scale),
                static_cast<float>(p.y * scale),
                static_cast<float>(p.z * scale)};
        };

        const auto n = std::array<float, 3>{
            static_cast<float>(triangle.normal.x),
            static_cast<float>(triangle.normal.y),
            static_cast<float>(triangle.normal.z)};
        const auto a = point(triangle.a);
        const auto b = point(triangle.b);
        const auto c = point(triangle.c);

        for (float value : n) writeFloatLe(output, value);
        for (float value : a) writeFloatLe(output, value);
        for (float value : b) writeFloatLe(output, value);
        for (float value : c) writeFloatLe(output, value);
        writeU16Le(output, 0);
    }

    return static_cast<bool>(output);
}

[[nodiscard]] bool writeAscii(
    const std::string& path,
    const std::vector<TriangleData>& triangles,
    double scale)
{
    std::ofstream output(path);
    if (!output)
        return false;

    output << std::setprecision(17);
    output << "solid MIR4D\n";
    for (const TriangleData& triangle : triangles)
    {
        const Point3 a{triangle.a.x * scale, triangle.a.y * scale, triangle.a.z * scale};
        const Point3 b{triangle.b.x * scale, triangle.b.y * scale, triangle.b.z * scale};
        const Point3 c{triangle.c.x * scale, triangle.c.y * scale, triangle.c.z * scale};

        output << "  facet normal " << triangle.normal.x << ' ' << triangle.normal.y << ' ' << triangle.normal.z << '\n';
        output << "    outer loop\n";
        output << "      vertex " << a.x << ' ' << a.y << ' ' << a.z << '\n';
        output << "      vertex " << b.x << ' ' << b.y << ' ' << b.z << '\n';
        output << "      vertex " << c.x << ' ' << c.y << ' ' << c.z << '\n';
        output << "    endloop\n";
        output << "  endfacet\n";
    }
    output << "endsolid MIR4D\n";
    return static_cast<bool>(output);
}
} // namespace

ExportResult StlExporter::exportTo(
    const std::string& path,
    const mir4d::Document& document,
    const ExportOptions& options)
{
    ExportResult result;
    result.format = options.binaryStl ? Format::StlBinary : Format::StlAscii;
    result.targetPath = path;

    const std::vector<TriangleData> triangles = collectTriangles(document, options);
    result.triangleCount = triangles.size();

    if (triangles.empty())
    {
        result.error = options.selectionOnly
            ? "No selected mesh triangles to export"
            : "Document contains no mesh triangles to export";
        return result;
    }

    const bool success = options.binaryStl
        ? writeBinary(path, triangles, options.unitScale)
        : writeAscii(path, triangles, options.unitScale);

    if (!success)
        result.error = "Cannot write STL file: " + path;

    return result;
}

} // namespace mir::io
