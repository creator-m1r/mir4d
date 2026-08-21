#include "StlImporter.hpp"

#include "../../Geometry/Tessellation/TriangleMesh.hpp"
#include "../../Math/Point.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace mir::io
{
namespace
{
struct VertexKey { std::uint64_t x; std::uint64_t y; std::uint64_t z; friend bool operator==(const VertexKey&, const VertexKey&) = default; };
struct VertexKeyHash
{
    std::size_t operator()(const VertexKey& key) const noexcept
    {
        auto mix = [](std::uint64_t value) noexcept { value ^= value >> 30; value *= 0xbf58476d1ce4e5b9ULL; value ^= value >> 27; value *= 0x94d049bb133111ebULL; value ^= value >> 31; return value; };
        return static_cast<std::size_t>(mix(key.x) ^ (mix(key.y) << 1) ^ (mix(key.z) << 2));
    }
};
struct Accumulator
{
    TriangleMesh3 mesh;
    std::unordered_map<VertexKey, std::size_t, VertexKeyHash> vertexMap;
    std::vector<Vector3> normalSums;
    double scale{1.0};
    bool join{true};

    std::size_t vertex(double x, double y, double z)
    {
        x *= scale; y *= scale; z *= scale;
        if (join)
        {
            const VertexKey key{std::bit_cast<std::uint64_t>(x), std::bit_cast<std::uint64_t>(y), std::bit_cast<std::uint64_t>(z)};
            const auto found = vertexMap.find(key);
            if (found != vertexMap.end()) return found->second;
            const std::size_t index = mesh.vertices.size();
            mesh.vertices.push_back({x, y, z}); normalSums.emplace_back(0.0, 0.0, 0.0); vertexMap.emplace(key, index); return index;
        }
        const std::size_t index = mesh.vertices.size(); mesh.vertices.push_back({x, y, z}); normalSums.emplace_back(0.0, 0.0, 0.0); return index;
    }

    void addTriangle(double nx, double ny, double nz, const std::array<std::array<double, 3>, 3>& points)
    {
        const std::size_t a = vertex(points[0][0], points[0][1], points[0][2]);
        const std::size_t b = vertex(points[1][0], points[1][1], points[1][2]);
        const std::size_t c = vertex(points[2][0], points[2][1], points[2][2]);
        mesh.triangles.push_back({a, b, c});
        if (normalSums.size() != mesh.vertices.size()) normalSums.resize(mesh.vertices.size(), Vector3{0.0, 0.0, 0.0});
        const double ax = mesh.vertices[b].x - mesh.vertices[a].x, ay = mesh.vertices[b].y - mesh.vertices[a].y, az = mesh.vertices[b].z - mesh.vertices[a].z;
        const double bx = mesh.vertices[c].x - mesh.vertices[a].x, by = mesh.vertices[c].y - mesh.vertices[a].y, bz = mesh.vertices[c].z - mesh.vertices[a].z;
        const double cx = ay * bz - az * by, cy = az * bx - ax * bz, cz = ax * by - ay * bx;
        const double length = std::sqrt(cx * cx + cy * cy + cz * cz);
        const double useNx = length > 1e-18 ? cx / length : nx, useNy = length > 1e-18 ? cy / length : ny, useNz = length > 1e-18 ? cz / length : nz;
        const Vector3 n{useNx, useNy, useNz}; normalSums[a] += n; normalSums[b] += n; normalSums[c] += n;
    }

    void finalizeNormals(bool generate)
    {
        if (!generate) return;
        mesh.normals.resize(mesh.vertices.size());
        for (std::size_t i = 0; i < normalSums.size(); ++i) mesh.normals[i] = normalSums[i].length() > 1e-18 ? normalSums[i].normalized() : Vector3{0.0, 0.0, 1.0};
    }
};

std::uint32_t readU32(const std::array<char, 4>& bytes)
{ return static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[0])) | (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[1])) << 8) | (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[2])) << 16) | (static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[3])) << 24); }
float readFloat(const std::array<char, 4>& bytes) { return std::bit_cast<float>(readU32(bytes)); }

bool isBinaryStl(const std::vector<char>& data)
{
    if (data.size() < 84) return false;
    std::array<char, 4> countBytes{}; std::copy_n(data.begin() + 80, 4, countBytes.begin());
    return 84ULL + static_cast<std::uint64_t>(readU32(countBytes)) * 50ULL == data.size();
}

ImportResult parseBinary(const std::string& path, const std::vector<char>& data, const ImportOptions& options)
{
    ImportResult result; result.format = Format::StlBinary; result.sourcePath = path;
    if (data.size() < 84) { result.error = "Binary STL header is incomplete"; return result; }
    std::array<char, 4> countBytes{}; std::copy_n(data.begin() + 80, 4, countBytes.begin());
    const std::uint32_t triangleCount = readU32(countBytes);
    Accumulator accumulator; accumulator.scale = options.unitScale; accumulator.join = options.joinIdenticalVertices;
    accumulator.mesh.vertices.reserve(static_cast<std::size_t>(triangleCount) * 3U); accumulator.mesh.triangles.reserve(triangleCount);
    std::size_t offset = 84;
    for (std::uint32_t i = 0; i < triangleCount; ++i)
    {
        if (offset + 50 > data.size()) { result.error = "Binary STL is truncated"; return result; }
        std::array<char, 4> nxBytes{}, nyBytes{}, nzBytes{}; std::copy_n(data.begin() + offset, 4, nxBytes.begin()); std::copy_n(data.begin() + offset + 4, 4, nyBytes.begin()); std::copy_n(data.begin() + offset + 8, 4, nzBytes.begin());
        std::array<std::array<double, 3>, 3> points{};
        for (int vertex = 0; vertex < 3; ++vertex)
        {
            const std::size_t base = offset + 12 + static_cast<std::size_t>(vertex) * 12; std::array<char, 4> x{}, y{}, z{};
            std::copy_n(data.begin() + base, 4, x.begin()); std::copy_n(data.begin() + base + 4, 4, y.begin()); std::copy_n(data.begin() + base + 8, 4, z.begin()); points[vertex] = {readFloat(x), readFloat(y), readFloat(z)};
        }
        accumulator.addTriangle(readFloat(nxBytes), readFloat(nyBytes), readFloat(nzBytes), points); offset += 50;
    }
    accumulator.finalizeNormals(options.generateNormals);
    if (!accumulator.mesh.isValid()) { result.error = "STL contains no valid triangles"; return result; }
    result.mesh = std::make_shared<TriangleMesh3>(std::move(accumulator.mesh)); result.boundsMin = result.mesh->boundsMin(); result.boundsMax = result.mesh->boundsMax(); result.triangleCount = result.mesh->triangles.size(); return result;
}

ImportResult parseAscii(const std::string& path, const std::vector<char>& data, const ImportOptions& options)
{
    ImportResult result; result.format = Format::StlAscii; result.sourcePath = path;
    std::istringstream stream(std::string(data.begin(), data.end())); std::string token; Accumulator accumulator; accumulator.scale = options.unitScale; accumulator.join = options.joinIdenticalVertices;
    bool inFacet = false; double nx = 0.0, ny = 0.0, nz = 1.0; std::array<std::array<double, 3>, 3> points{}; int pointCount = 0;
    while (stream >> token)
    {
        if (token == "facet") { std::string normal; stream >> normal; if (normal != "normal") continue; stream >> nx >> ny >> nz; inFacet = true; pointCount = 0; continue; }
        if (token == "vertex" && inFacet && pointCount < 3) { stream >> points[pointCount][0] >> points[pointCount][1] >> points[pointCount][2]; ++pointCount; if (pointCount == 3) { accumulator.addTriangle(nx, ny, nz, points); inFacet = false; } }
    }
    accumulator.finalizeNormals(options.generateNormals); if (!accumulator.mesh.isValid()) { result.error = "ASCII STL contains no valid facets"; return result; }
    result.mesh = std::make_shared<TriangleMesh3>(std::move(accumulator.mesh)); result.boundsMin = result.mesh->boundsMin(); result.boundsMax = result.mesh->boundsMax(); result.triangleCount = result.mesh->triangles.size(); return result;
}
}

ImportResult StlImporter::importFile(const std::string& path, const ImportOptions& options) const
{
    ImportResult result; result.sourcePath = path; std::ifstream file(path, std::ios::binary); if (!file) { result.error = "Cannot open STL file: " + path; return result; }
    const std::vector<char> data{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()}; if (data.empty()) { result.error = "STL file is empty"; return result; }
    return isBinaryStl(data) ? parseBinary(path, data, options) : parseAscii(path, data, options);
}

}
