// MirEngine/Scene/Node.h
// =================================================================================
// Узел графа сцены.
//
// Хранит локальную трансформацию (положение, масштаб; поворот будет добавлен позже),
// ссылку на геометрию (Mesh) и список дочерних узлов. Образует иерархическое
// дерево сцены, которое обходится рендерером для формирования команд рисования.
//
// Архитектура:
//   - Не зависит от рендеринг-бэкенда. Только математика и структура.
//   - Может быть как групповым (без геометрии), так и терминальным (с Mesh).
//   - Для рендеринга внешний код запрашивает мировую матрицу через getWorldMatrix(),
//     рекурсивно перемножая локальные матрицы от корня к узлу.
//
// Использование:
//   Node root;
//   Node* child = root.addChild();
//   child->setMesh(someMesh);
//   child->setPosition(1,0,0);
//   // при обходе: Matrix4 world = parentWorld * child->getLocalMatrix();
// =================================================================================

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <array>
#include "../Core/Math/MathTypes.h"   // Vector3, Matrix4Raw (или временные)

namespace MirEngine {

// Временные математические типы (будут заменены на Core/Math)
using Vector3 = struct { float x, y, z; };
using Matrix4Raw = std::array<float, 16>;

class Mesh; // предварительное объявление

class Node {
public:
    Node(const std::string& name = "");
    ~Node();

    // --------------------------------------------------------------------------
    // Иерархия
    // --------------------------------------------------------------------------
    Node* addChild(std::unique_ptr<Node> child);
    void removeChild(Node* child);
    Node* getParent() const { return m_parent; }
    const std::vector<std::unique_ptr<Node>>& getChildren() const { return m_children; }

    // --------------------------------------------------------------------------
    // Геометрия
    // --------------------------------------------------------------------------
    void setMesh(std::shared_ptr<Mesh> mesh) { m_mesh = mesh; }
    Mesh* getMesh() const { return m_mesh.get(); }

    // --------------------------------------------------------------------------
    // Трансформация
    // --------------------------------------------------------------------------
    void setPosition(float x, float y, float z) { m_position = {x, y, z}; m_localDirty = true; }
    void setScale(float x, float y, float z) { m_scale = {x, y, z}; m_localDirty = true; }

    // Прямая установка локальной матрицы (например, из Assimp)
    void setLocalMatrix(const Matrix4Raw& mat) {
        m_cachedLocalMatrix = mat;
        m_localDirty = false;
    }

    Vector3 getPosition() const { return m_position; }
    Vector3 getScale() const { return m_scale; }

    // Локальная матрица (T*R*S, пока без вращения)
    Matrix4Raw getLocalMatrix() const;

    // Мировая матрица (рекурсивно перемножает локальные матрицы от корня)
    Matrix4Raw getWorldMatrix() const;

    // Имя узла (для отладки)
    const std::string& getName() const { return m_name; }

private:
    std::string m_name;
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children;

    std::shared_ptr<Mesh> m_mesh;

    Vector3 m_position = {0, 0, 0};
    Vector3 m_scale    = {1, 1, 1};

    mutable bool m_localDirty = true;
    mutable Matrix4Raw m_cachedLocalMatrix; // кэш

    // Внутренний метод обновления локальной матрицы
    void updateLocalMatrix() const;
};

} // namespace MirEngine