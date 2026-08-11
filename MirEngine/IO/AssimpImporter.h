// MirEngine/IO/AssimpImporter.h
// =================================================================================
// Реализация импортёра через библиотеку Assimp.
// Поддерживает форматы: STEP, IGES, OBJ, STL, glTF, FBX и многие другие.
// =================================================================================

#pragma once

#include "Importer.h"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace MirEngine {

class AssimpImporter : public Importer {
public:
    AssimpImporter();
    ~AssimpImporter() override;

    std::unique_ptr<Node> load(const std::string& filePath) override;

private:
    // Рекурсивно обходит граф Assimp и строит Node-дерево
    void processNode(const aiScene* scene, const aiNode* aiNode, Node* parentNode);

    // Конвертирует Assimp-меш в наш Mesh
    std::shared_ptr<Mesh> convertMesh(const aiScene* scene, const aiMesh* aiMesh);
};

} // namespace MirEngine