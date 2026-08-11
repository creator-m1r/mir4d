// MirEngine/Geometry/Topology/Vertex.hpp
// 📍 Топологическая вершина (Vertex) — узел графа геометрии.
//
// Vertex — это самая маленькая, но очень важная часть топологического
// представления (B-Rep). Каждая вершина — это просто точка в пространстве,
// но она «знает», какие рёбра из неё выходят. Именно через вершины
// рёбра соединяются друг с другом, образуя грани и, в конечном счёте,
// всё твёрдое тело.
//
// Представь себе каркас из проволочек (рёбер), спаянных в узелках (вершинах).
// Vertex хранит координаты узелка и список проволочек, которые к нему
// прикреплены.
//
// Vertex используется ВЕЗДЕ в топологии:
//   • Edge ссылается на две вершины (начало и конец).
//   • Loop обходит вершины, проверяя, что рёбра стыкуются.
//   • Face опирается на вершины для вычисления нормали и формы.
//   • Body собирает все вершины в общий список.
//
// Чистый C++23, без внешних зависимостей.


#pragma once

#include <vector>

#include "../Core/Identity/TypedID.hpp"
#include "../Geometry/Point/Point3.hpp"

namespace mir
{

class Vertex
{
public:

    Vertex() = default;

    explicit Vertex(
        VertexID id,
        const Point3& point)
        : m_id(id),
          m_point(point)
    {
    }

    VertexID id() const noexcept
    {
        return m_id;
    }

    const Point3& point() const noexcept
    {
        return m_point;
    }

    void setPoint(const Point3& point)
    {
        m_point = point;
    }

    const std::vector<EdgeID>& edges() const noexcept
    {
        return m_edges;
    }

    void addEdge(EdgeID id)
    {
        m_edges.push_back(id);
    }

private:

    VertexID m_id;

    Point3 m_point;

    std::vector<EdgeID> m_edges;
};

}