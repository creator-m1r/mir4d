// MirEngine/Scene/Scene.h
// =================================================================================
// Контейнер сцены — владеет иерархией узлов (Node).
// =================================================================================
#pragma once

#include <memory>
#include "Node.h"

namespace MirEngine {

class Scene {
public:
    Scene();
    ~Scene();

    Node* getRoot() const { return m_root.get(); }

    // Добавляет узел в корень сцены
    Node* addNode(std::unique_ptr<Node> node);

    // Вспомогательная: создать узел с мешем на корневом уровне
    Node* addMeshNode(const std::string& name, std::shared_ptr<Mesh> mesh);

private:
    std::unique_ptr<Node> m_root;
};

} // namespace MirEngine