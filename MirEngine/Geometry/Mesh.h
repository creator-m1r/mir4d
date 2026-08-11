// MirEngine/Geometry/Mesh.h
// =================================================================================
// Mesh — полигональная сетка (геометрия) для визуализации.
//
// Хранит массивы вершин (Vertex) и индексов (uint32_t), описывающие дискретную
// геометрию. Не зависит от GPU или рендеринг-бэкенда; GPU-ресурсы создаются
// отдельно через RenderDevice и кэшируются.
//
// В CAD-контексте Mesh может представлять как аналитические поверхности (после
// триангуляции), так и импортированные данные (STL, OBJ).
//
// Архитектура:
//   - Mesh создаётся процедурно или загружается из файла.
//   - Хранит данные в оперативной памяти (CPU).
//   - По запросу Renderer создаёт соответствующий VertexArray и IndexBuffer.
//   - Может быть общим для нескольких узлов (instancing) — но пока без этого.
// =================================================================================

#pragma once

#include <vector>
#include <cstdint>
#include "../Rendering/Resources/Vertex.h"  // Структура Vertex

namespace MirEngine {

class Mesh {
public:
    Mesh() = default;

    // --------------------------------------------------------------------------
    // Создаёт геометрию из готовых массивов.
    // --------------------------------------------------------------------------
    void setGeometry(const std::vector<Rendering::Vertex>& vertices,
                     const std::vector<uint32_t>& indices) {
        m_vertices = vertices;
        m_indices = indices;
    }

    // --------------------------------------------------------------------------
    // Доступ к данным (CPU)
    // --------------------------------------------------------------------------
    const std::vector<Rendering::Vertex>& getVertices() const { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const { return m_indices; }

    size_t vertexCount() const { return m_vertices.size(); }
    size_t indexCount() const  { return m_indices.size(); }

    // --------------------------------------------------------------------------
    // Фабричные методы для базовых CAD-примитивов.
    // Будут реализованы в MeshPrimitives.cpp (позже).
    // --------------------------------------------------------------------------
    static Mesh createCube(float size = 1.0f);
    static Mesh createSphere(float radius = 0.5f, uint32_t segments = 16);
    static Mesh createCylinder(float radius = 0.5f, float height = 1.0f, uint32_t segments = 16);

private:
    std::vector<Rendering::Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

} // namespace MirEngine
