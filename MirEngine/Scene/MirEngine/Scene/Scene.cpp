// MirEngine/Scene/Scene.cpp
#include "Scene.h"

namespace MirEngine {

Scene::Scene() : m_root(std::make_unique<Node>("Root")) {}

Scene::~Scene() = default;

Node* Scene::addNode(std::unique_ptr<Node> node) {
    return m_root->addChild(std::move(node));
}

Node* Scene::addMeshNode(const std::string& name, std::shared_ptr<Mesh> mesh) {
    auto node = std::make_unique<Node>(name);
    node->setMesh(mesh);
    return addNode(std::move(node));
}

} // namespace MirEngine