// MirEngine/IO/AssimpImporter.cpp
// =================================================================================
#include "AssimpImporter.h"
#include "../Scene/Node.h"
#include "../Geometry/Mesh.h"
#include "../Rendering/Resources/Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>

namespace MirEngine {

AssimpImporter::AssimpImporter() {}
AssimpImporter::~AssimpImporter() {}

std::unique_ptr<Node> AssimpImporter::load(const std::string& filePath) {
    Assimp::Importer importer;
    // Параметры импорта: триангуляция, генерация нормалей, переворот UV (если нужно)
    const unsigned int flags = aiProcess_Triangulate |
                               aiProcess_GenNormals |
                               aiProcess_FlipUVs;
    const aiScene* scene = importer.ReadFile(filePath, flags);
    if (!scene || !scene->mRootNode) {
        spdlog::error("[AssimpImporter] Failed to load '{}': {}", filePath, importer.GetErrorString());
        return nullptr;
    }

    auto rootNode = std::make_unique<Node>("Root_Imported");
    processNode(scene, scene->mRootNode, rootNode.get());
    spdlog::info("[AssimpImporter] Loaded '{}': {} meshes, {} nodes.", 
                 filePath, scene->mNumMeshes, 1); // кол-во узлов можно подсчитать
    return rootNode;
}

void AssimpImporter::processNode(const aiScene* scene, const aiNode* aiNode, Node* parentNode) {
    // Создаём узел для текущего aiNode
    auto ourNode = std::make_unique<Node>(aiNode->mName.C_Str());

    // Устанавливаем трансформацию из матрицы Assimp (row-major -> column-major)
    aiMatrix4x4 t = aiNode->mTransformation;
    Matrix4Raw mat = {
        t.a1, t.b1, t.c1, t.d1,
        t.a2, t.b2, t.c2, t.d2,
        t.a3, t.b3, t.c3, t.d3,
        t.a4, t.b4, t.c4, t.d4
    };
    ourNode->setLocalMatrix(mat); // нужно добавить метод setLocalMatrix в Node (см. ниже)

    // Прикрепляем меши этого узла
    for (unsigned int i = 0; i < aiNode->mNumMeshes; ++i) {
        unsigned int meshIndex = aiNode->mMeshes[i];
        const aiMesh* aiMesh = scene->mMeshes[meshIndex];
        auto mesh = convertMesh(scene, aiMesh);
        if (mesh) {
            ourNode->setMesh(mesh);
        }
    }

    // Рекурсивно обходим дочерние узлы
    for (unsigned int i = 0; i < aiNode->mNumChildren; ++i) {
        processNode(scene, aiNode->mChildren[i], ourNode.get());
    }

    parentNode->addChild(std::move(ourNode));
}

std::shared_ptr<Mesh> AssimpImporter::convertMesh(const aiScene* /*scene*/, const aiMesh* aiMesh) {
    if (!aiMesh->HasPositions()) {
        spdlog::warn("[AssimpImporter] Mesh '{}' has no vertices, skipping.", aiMesh->mName.C_Str());
        return nullptr;
    }

    std::vector<Rendering::Vertex> vertices;
    vertices.reserve(aiMesh->mNumVertices);
    for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i) {
        Rendering::Vertex v;
        v.position = {aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z};
        if (aiMesh->HasNormals()) {
            v.normal = {aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z};
        }
        if (aiMesh->HasTextureCoords(0)) {
            v.uv = {aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y};
        }
        vertices.push_back(v);
    }

    std::vector<uint32_t> indices;
    for (unsigned int f = 0; f < aiMesh->mNumFaces; ++f) {
        const aiFace& face = aiMesh->mFaces[f];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->setGeometry(vertices, indices);
    return mesh;
}

} // namespace MirEngine