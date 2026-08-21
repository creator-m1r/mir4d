#include "AssimpImporter.hpp"

#include "../ImportOptions.hpp"
#include "../ImportService.hpp"

#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>

namespace mir::io
{
namespace
{

struct Accumulator
{
    TriangleMesh3 mesh;

    void append(const aiScene& scene,
                const aiNode& node,
                const aiMatrix4x4& parentTransform,
                bool generateNormals,
                double unitScale)
    {
        const aiMatrix4x4 transform = parentTransform * node.mTransformation;

        for (unsigned int meshIndex = 0; meshIndex < node.mNumMeshes; ++meshIndex)
        {
            const aiMesh* source = scene.mMeshes[node.mMeshes[meshIndex]];
            if (!source || !source->HasPositions())
                continue;

            const std::size_t base = mesh.vertices.size();
            mesh.vertices.reserve(base + source->mNumVertices);

            for (unsigned int vertexIndex = 0; vertexIndex < source->mNumVertices; ++vertexIndex)
            {
                const aiVector3D position = transform * source->mVertices[vertexIndex];
                mesh.vertices.push_back({
                    static_cast<double>(position.x) * unitScale,
                    static_cast<double>(position.y) * unitScale,
                    static_cast<double>(position.z) * unitScale});

                if (source->HasNormals())
                {
                    const aiVector3D normal = transform * source->mNormals[vertexIndex];
                    mesh.normals.push_back({
                        static_cast<double>(normal.x),
                        static_cast<double>(normal.y),
                        static_cast<double>(normal.z)});
                }
                else if (generateNormals)
                {
                    mesh.normals.push_back(Vector3::zero());
                }
            }

            for (unsigned int faceIndex = 0; faceIndex < source->mNumFaces; ++faceIndex)
            {
                const aiFace& face = source->mFaces[faceIndex];
                if (face.mNumIndices != 3)
                    continue;

                mesh.triangles.push_back({
                    base + face.mIndices[0],
                    base + face.mIndices[1],
                    base + face.mIndices[2]});
            }
        }

        for (unsigned int childIndex = 0; childIndex < node.mNumChildren; ++childIndex)
        {
            if (node.mChildren[childIndex])
                append(scene, *node.mChildren[childIndex], transform, generateNormals, unitScale);
        }
    }
};

void generateNormals(TriangleMesh3& mesh)
{
    if (mesh.normals.size() != mesh.vertices.size())
        mesh.normals.assign(mesh.vertices.size(), Vector3::zero());
    else
        std::fill(mesh.normals.begin(), mesh.normals.end(), Vector3::zero());

    for (const TriangleMesh3::Triangle& triangle : mesh.triangles)
    {
        const Point3& a = mesh.vertices[triangle.a];
        const Point3& b = mesh.vertices[triangle.b];
        const Point3& c = mesh.vertices[triangle.c];
        const Vector3 ab = b - a;
        const Vector3 ac = c - a;
        const Vector3 normal = Vector3::cross(ab, ac).normalized();
        mesh.normals[triangle.a] += normal;
        mesh.normals[triangle.b] += normal;
        mesh.normals[triangle.c] += normal;
    }

    for (Vector3& normal : mesh.normals)
        normal = normal.normalized();
}

} // namespace

ImportResult AssimpImporter::importFile(
    const std::string& path,
    const ImportOptions& options) const
{
    ImportResult result;
    result.sourcePath = path;
    result.format = ImportService::detectFormat(path);

    Assimp::Importer importer;
    unsigned int flags = aiProcess_Triangulate;
    if (options.joinIdenticalVertices)
        flags |= aiProcess_JoinIdenticalVertices;
    if (options.generateNormals)
        flags |= aiProcess_GenNormals;

    const aiScene* scene = importer.ReadFile(path, flags);
    if (!scene || !scene->mRootNode)
    {
        result.error = importer.GetErrorString();
        return result;
    }

    Accumulator accumulator;
    accumulator.append(
        *scene,
        *scene->mRootNode,
        aiMatrix4x4(),
        options.generateNormals,
        options.unitScale);

    if (options.generateNormals && !accumulator.mesh.hasVertexNormals())
        generateNormals(accumulator.mesh);

    if (!accumulator.mesh.isValid())
    {
        result.error = "Assimp produced no valid triangle mesh.";
        return result;
    }

    result.mesh = std::make_shared<TriangleMesh3>(std::move(accumulator.mesh));
    result.triangleCount = result.mesh->triangles.size();
    result.boundsMin = result.mesh->boundsMin();
    result.boundsMax = result.mesh->boundsMax();
    return result;
}

namespace
{
struct AssimpRegistrar
{
    AssimpRegistrar()
    {
        using namespace mir::io;
        const auto assimpImporter = [](const std::string& path, const ImportOptions& options)
        {
            return AssimpImporter{}.importFile(path, options);
        };
        ImportService::registerImporter(Format::Obj, assimpImporter);
        ImportService::registerImporter(Format::Ply, assimpImporter);
        ImportService::registerImporter(Format::Gltf, assimpImporter);
        ImportService::registerImporter(Format::Glb, assimpImporter);
        ImportService::registerImporter(Format::Fbx, assimpImporter);
    }
};

static const AssimpRegistrar g_assimpRegistrar;
} // namespace

} // namespace mir::io
