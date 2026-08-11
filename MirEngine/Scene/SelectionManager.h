// MirEngine/Scene/SelectionManager.h
// =================================================================================
// Менеджер выделения объектов сцены.
//
// Хранит множество выделенных узлов. Предоставляет методы для добавления/удаления
// выделения, проверки, очистки. Также выполняет ray casting (software picking) 
// для поиска пересечений луча мыши с мешами сцены.
//
// Архитектура:
//   - Не зависит от рендеринг-бэкенда.
//   - Получает луч в мировом пространстве (начало + направление).
//   - Возвращает ближайший узел, чья геометрия пересекает луч.
//   - Может использоваться как для кликов мыши, так и для множественного выделения.
// =================================================================================

#pragma once

#include <unordered_set>
#include <memory>
#include "../Core/Math/MathTypes.h"  // Vector3

namespace MirEngine {

class Node;
class Scene;

class SelectionManager {
public:
    SelectionManager() = default;

    // --------------------------------------------------------------------------
    // Текущее выделение
    // --------------------------------------------------------------------------
    const std::unordered_set<Node*>& getSelection() const { return m_selectedNodes; }
    bool isSelected(Node* node) const { return m_selectedNodes.count(node) > 0; }

    void select(Node* node);
    void deselect(Node* node);
    void toggle(Node* node);
    void clear();

    // --------------------------------------------------------------------------
    // Ray casting: поиск ближайшего узла, пересекающего луч.
    // origin - начало луча в мировых координатах.
    // direction - нормированное направление луча.
    // Возвращает ближайший узел (или nullptr).
    // distanceOut - опционально: расстояние до точки пересечения.
    // --------------------------------------------------------------------------
    Node* pick(Scene& scene, const Vector3& origin, const Vector3& direction,
               float* distanceOut = nullptr);

    // --------------------------------------------------------------------------
    // Выделение по клику: если попали — выделяем, иначе снимаем.
    // --------------------------------------------------------------------------
    void pickAndSelect(Scene& scene, const Vector3& origin, const Vector3& direction,
                       bool addToSelection = false);

private:
    std::unordered_set<Node*> m_selectedNodes;

    // Проверка пересечения луча с треугольником (алгоритм Möller–Trumbore)
    bool rayIntersectsTriangle(const Vector3& orig, const Vector3& dir,
                               const Vector3& v0, const Vector3& v1, const Vector3& v2,
                               float& t) const;
    // Проверка пересечения луча с мешем (все треугольники)
    bool rayIntersectsMesh(const class Mesh* mesh, const Matrix4Raw& worldMatrix,
                           const Vector3& orig, const Vector3& dir, float& t) const;
};

} // namespace MirEngine