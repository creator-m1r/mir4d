// MirEngine/Geometry/Topology/Edge.hpp
// ➖ Топологическое ребро (Edge) — связь между двумя вершинами в графе геометрии.
//
// Edge — это фундаментальный элемент граничного представления (B-Rep).
// Он соединяет две вершины (Vertex) и является частью одного или двух
// замкнутых контуров (Wire), ограничивающих грани (Face). В отличие от
// геометрического отрезка (Segment3), Edge может быть искривлённым,
// если с ним связана параметрическая кривая (ParametricCurve).
//
// Основные свойства:
//   • v0, v1       — индексы вершин (Vertex), которые соединяет ребро.
//   • curve        — опциональная кривая, задающая точную геометрию ребра.
//   • faces        — индексы граней (Face), примыкающих к этому ребру.
//   • length()     — длина ребра (по кривой или по прямой между вершинами).
//   • pointAt(t)   — точка на ребре в зависимости от параметра t.
//   • tangentAt(t) — касательный вектор вдоль ребра.
//
// Edge используется в:
//   • Body (B-Rep) — как часть граней.
//   • Wire (контур) — замкнутая или разомкнутая цепочка рёбер.
//   • Эскизах (Sketch) — как примитив для выдавливания.
//   • Алгоритмах поиска кратчайшего пути по сетке.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include <vector>

#include "../Core/Identity/TypedID.hpp"

namespace mir
{

class Edge
{
public:

    Edge() = default;

    explicit Edge(EdgeID id)
        : m_id(id)
    {
    }

    EdgeID id() const noexcept
    {
        return m_id;
    }

    VertexID startVertex() const noexcept
    {
        return m_startVertex;
    }

    VertexID endVertex() const noexcept
    {
        return m_endVertex;
    }

    void setStartVertex(VertexID id) noexcept
    {
        m_startVertex = id;
    }

    void setEndVertex(VertexID id) noexcept
    {
        m_endVertex = id;
    }

    const std::vector<LoopID>& loops() const noexcept
    {
        return m_loops;
    }

    void addLoop(LoopID id)
    {
        m_loops.push_back(id);
    }

private:

    EdgeID m_id;

    VertexID m_startVertex;
    VertexID m_endVertex;

    std::vector<LoopID> m_loops;
};

}