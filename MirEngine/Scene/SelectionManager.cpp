// MirEngine/Scene/SelectionManager.cpp
#include "SelectionManager.h"
#include "Scene.h"
#include "Node.h"
#include "../Geometry/Mesh.h"
#include <limits>

namespace MirEngine {

void SelectionManager::select(Node* node) { if (node) m_selectedNodes.insert(node); }
void SelectionManager::deselect(Node* node) { m_selectedNodes.erase(node); }
void SelectionManager::toggle(Node* node) {
    if (isSelected(node)) deselect(node); else select(node);
}
void SelectionManager::clear() { m_selectedNodes.clear(); }

Node* SelectionManager::pick(Scene& scene, const Vector3& origin, const Vector3& direction,
                             float* distanceOut) {
    float closestT = std::numeric_limits<float>::max();
    Node* closestNode = nullptr;

    // Рекурсивный обход сцены
    std::function<void(Node*, const Matrix4Raw&)> traverse = 
    [&](Node* node, const Matrix4Raw& parentWorld) {
        Matrix4Raw world = node->getWorldMatrix();
        // Если есть меш, проверяем пересечение
        if (Mesh* mesh = node->getMesh()) {
            float t;
            if (rayIntersectsMesh(mesh, world, origin, direction, t)) {
                if (t < closestT) {
                    closestT = t;
                    closestNode = node;
                }
            }
        }
        // Рекурсия для детей
        for (auto& child : node->getChildren()) {
            traverse(child.get(), world);
        }
    };

    traverse(scene.getRoot(), IdentityMatrix4());

    if (distanceOut && closestNode) *distanceOut = closestT;
    return closestNode;
}

void SelectionManager::pickAndSelect(Scene& scene, const Vector3& origin,
                                     const Vector3& direction, bool addToSelection) {
    Node* node = pick(scene, origin, direction);
    if (addToSelection) {
        if (node) toggle(node);
    } else {
        clear();
        if (node) select(node);
    }
}

// ---------------------------------------------------------------------------------
// Проверка пересечения луча с треугольником (Möller–Trumbore)
// ---------------------------------------------------------------------------------
bool SelectionManager::rayIntersectsTriangle(const Vector3& orig, const Vector3& dir,
                                             const Vector3& v0, const Vector3& v1,
                                             const Vector3& v2, float& t) const {
    const float EPSILON = 1e-6f;
    Vector3 edge1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
    Vector3 edge2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
    Vector3 h = {dir.y * edge2.z - dir.z * edge2.y,
                 dir.z * edge2.x - dir.x * edge2.z,
                 dir.x * edge2.y - dir.y * edge2.x};
    float a = edge1.x * h.x + edge1.y * h.y + edge1.z * h.z;
    if (std::abs(a) < EPSILON) return false;

    float f = 1.0f / a;
    Vector3 s = {orig.x - v0.x, orig.y - v0.y, orig.z - v0.z};
    float u = f * (s.x * h.x + s.y * h.y + s.z * h.z);
    if (u < 0.0f || u > 1.0f) return false;

    Vector3 q = {s.y * edge1.z - s.z * edge1.y,
                 s.z * edge1.x - s.x * edge1.z,
                 s.x * edge1.y - s.y * edge1.x};
    float v = f * (dir.x * q.x + dir.y * q.y + dir.z * q.z);
    if (v < 0.0f || u + v > 1.0f) return false;

    t = f * (edge2.x * q.x + edge2.y * q.y + edge2.z * q.z);
    return t > EPSILON;
}

bool SelectionManager::rayIntersectsMesh(const Mesh* mesh, const Matrix4Raw& world,
                                         const Vector3& orig, const Vector3& dir,
                                         float& t) const {
    if (!mesh) return false;
    float minT = std::numeric_limits<float>::max();
    bool hit = false;
    const auto& vertices = mesh->getVertices();
    const auto& indices = mesh->getIndices();

    // Преобразуем луч в локальное пространство меша (если нужна точность; 
    // для простоты можно предположить, что матрица уже учтена, но здесь 
    // преобразуем вершины в мировое пространство)
    // Для упрощения будем считать, что world не сильно искажает, и проверяем
    // в мировых координатах, умножая каждую вершину. Это дорого, но для CAD
    // допустимо.
    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        uint32_t i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];
        // Преобразуем вершины в мировые (умножение матрицы на вектор)
        auto transform = [&](const Rendering::Vector3& v) -> Vector3 {
            float x = world[0]*v.x + world[4]*v.y + world[8]*v.z  + world[12];
            float y = world[1]*v.x + world[5]*v.y + world[9]*v.z  + world[13];
            float z = world[2]*v.x + world[6]*v.y + world[10]*v.z + world[14];
            return {x, y, z};
        };
        Vector3 v0 = transform(vertices[i0].position);
        Vector3 v1 = transform(vertices[i1].position);
        Vector3 v2 = transform(vertices[i2].position);
        float triT;
        if (rayIntersectsTriangle(orig, dir, v0, v1, v2, triT)) {
            if (triT < minT) {
                minT = triT;
                hit = true;
            }
        }
    }
    if (hit) t = minT;
    return hit;
}

} // namespace MirEngine