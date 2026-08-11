// MirEngine/Scene/Node.cpp
// =================================================================================
#include "Node.h"
#include <algorithm>

namespace MirEngine {

// Вспомогательные функции для построения матриц (позже заменим на Math)
static Matrix4Raw identityMatrix4() {
    return {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
}

static Matrix4Raw translate(const Vector3& v) {
    auto m = identityMatrix4();
    m[12] = v.x; m[13] = v.y; m[14] = v.z;
    return m;
}

static Matrix4Raw scaleMatrix(const Vector3& v) {
    auto m = identityMatrix4();
    m[0] = v.x; m[5] = v.y; m[10] = v.z;
    return m;
}

static Matrix4Raw multiply(const Matrix4Raw& a, const Matrix4Raw& b) {
    Matrix4Raw r;
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
                sum += a[row + k*4] * b[k + col*4];
            r[row + col*4] = sum;
        }
    }
    return r;
}

// ---------------------------------------------------------------------------------
Node::Node(const std::string& name) : m_name(name) {
    m_cachedLocalMatrix = identityMatrix4();
}

Node::~Node() {}

Node* Node::addChild(std::unique_ptr<Node> child) {
    child->m_parent = this;
    Node* ptr = child.get();
    m_children.push_back(std::move(child));
    return ptr;
}

void Node::removeChild(Node* child) {
    auto it = std::find_if(m_children.begin(), m_children.end(),
                           [child](const std::unique_ptr<Node>& c) { return c.get() == child; });
    if (it != m_children.end()) {
        (*it)->m_parent = nullptr;
        m_children.erase(it);
    }
}

void Node::updateLocalMatrix() const {
    if (!m_localDirty) return;
    Matrix4Raw T = translate(m_position);
    Matrix4Raw S = scaleMatrix(m_scale);
    // Локальная матрица = T * S (без поворота)
    m_cachedLocalMatrix = multiply(T, S);
    m_localDirty = false;
}

Matrix4Raw Node::getLocalMatrix() const {
    updateLocalMatrix();
    return m_cachedLocalMatrix;
}

Matrix4Raw Node::getWorldMatrix() const {
    Matrix4Raw local = getLocalMatrix();
    if (m_parent) {
        return multiply(m_parent->getWorldMatrix(), local);
    }
    return local;
}

} // namespace MirEngine