// MirEngine/Geometry/MeshPrimitives.cpp
// =================================================================================
// Фабрики создания базовых геометрических примитивов для CAD-сцены.
// Генерируют вершины и индексы, готовые для загрузки в GPU через RenderDevice.
// =================================================================================

#include "Mesh.h"
#include <cmath>
#include <cstdint>

namespace MirEngine {

// ---------------------------------------------------------------------------------
// Куб (бокс) с центром в начале координат, размером side^3.
// Вершины: 24 (по 4 на грань, с нормалями, направленными наружу).
// Индексы: 36 (6 граней * 2 треугольника * 3 индекса).
// ---------------------------------------------------------------------------------
Mesh Mesh::createCube(float size) {
    Mesh mesh;
    float s = size * 0.5f;
    std::vector<Rendering::Vertex> verts;
    std::vector<uint32_t> indices;

    // Грани: перед, зад, лево, право, верх, низ
    struct Face { Rendering::Vector3 n; Rendering::Vector3 p[4]; Rendering::Vector2 uv[4]; };
    Face faces[6] = {
        {{ 0, 0, 1}, {{-s,-s, s},{ s,-s, s},{ s, s, s},{-s, s, s}}, {{0,0},{1,0},{1,1},{0,1}}}, // front
        {{ 0, 0,-1}, {{ s,-s,-s},{-s,-s,-s},{-s, s,-s},{ s, s,-s}}, {{0,0},{1,0},{1,1},{0,1}}}, // back
        {{-1, 0, 0}, {{-s,-s,-s},{-s,-s, s},{-s, s, s},{-s, s,-s}}, {{0,0},{1,0},{1,1},{0,1}}}, // left
        {{ 1, 0, 0}, {{ s,-s, s},{ s,-s,-s},{ s, s,-s},{ s, s, s}}, {{0,0},{1,0},{1,1},{0,1}}}, // right
        {{ 0, 1, 0}, {{-s, s, s},{ s, s, s},{ s, s,-s},{-s, s,-s}}, {{0,0},{1,0},{1,1},{0,1}}}, // top
        {{ 0,-1, 0}, {{-s,-s,-s},{ s,-s,-s},{ s,-s, s},{-s,-s, s}}, {{0,0},{1,0},{1,1},{0,1}}}  // bottom
    };

    for (int f = 0; f < 6; ++f) {
        uint32_t base = static_cast<uint32_t>(verts.size());
        for (int v = 0; v < 4; ++v) {
            verts.push_back({faces[f].p[v], faces[f].n, faces[f].uv[v]});
        }
        indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
    }

    mesh.setGeometry(verts, indices);
    return mesh;
}

// ---------------------------------------------------------------------------------
// Сфера (аппроксимация UV-сферой) с центром в начале координат.
// segments - количество сегментов по долготе и широте.
// ---------------------------------------------------------------------------------
Mesh Mesh::createSphere(float radius, uint32_t segments) {
    Mesh mesh;
    std::vector<Rendering::Vertex> verts;
    std::vector<uint32_t> indices;

    uint32_t stacks = segments;
    uint32_t slices = segments * 2;

    for (uint32_t i = 0; i <= stacks; ++i) {
        float phi = 3.14159265f * i / stacks; // [0..PI]
        float y = radius * cosf(phi);
        float r = radius * sinf(phi);
        for (uint32_t j = 0; j <= slices; ++j) {
            float theta = 2.0f * 3.14159265f * j / slices;
            Rendering::Vector3 pos = {r * cosf(theta), y, r * sinf(theta)};
            Rendering::Vector3 norm = {pos.x / radius, pos.y / radius, pos.z / radius};
            Rendering::Vector2 uv = {static_cast<float>(j) / slices, static_cast<float>(i) / stacks};
            verts.push_back({pos, norm, uv});
        }
    }

    for (uint32_t i = 0; i < stacks; ++i) {
        for (uint32_t j = 0; j < slices; ++j) {
            uint32_t a = i * (slices + 1) + j;
            uint32_t b = a + slices + 1;
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);
            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }

    mesh.setGeometry(verts, indices);
    return mesh;
}

// ---------------------------------------------------------------------------------
// Цилиндр (без крышек) с центром в начале координат, ось Y.
// ---------------------------------------------------------------------------------
Mesh Mesh::createCylinder(float radius, float height, uint32_t segments) {
    Mesh mesh;
    std::vector<Rendering::Vertex> verts;
    std::vector<uint32_t> indices;
    float halfH = height * 0.5f;

    // Боковая поверхность (два ряда вершин: верхнее и нижнее кольцо)
    for (uint32_t i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.14159265f * i / segments;
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        Rendering::Vector3 norm = {cosf(theta), 0.0f, sinf(theta)};
        Rendering::Vector2 uv = {static_cast<float>(i) / segments, 0.0f};

        // Нижняя точка
        verts.push_back({{x, -halfH, z}, norm, uv});
        // Верхняя точка
        verts.push_back({{x,  halfH, z}, norm, {uv.x, 1.0f}});
    }

    for (uint32_t i = 0; i < segments; ++i) {
        uint32_t base = i * 2;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }

    mesh.setGeometry(verts, indices);
    return mesh;
}

} // namespace MirEngine