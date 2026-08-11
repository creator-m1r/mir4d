#pragma once

#include <unordered_map>

#include "../Core/Identity/TypedID.hpp"

#include "../Geometry/Topology/Vertex.hpp"
#include "../Geometry/Topology/Edge.hpp"
#include "../Geometry/Topology/Loop.hpp"
#include "../Geometry/Topology/Face.hpp"
#include "../Geometry/Topology/Shell.hpp"
#include "../Geometry/Topology/Solid.hpp"
#include "../Geometry/Topology/Body.hpp"

namespace mir
{

class TopologyStore
{
public:

    VertexID createVertex(const Point3& point)
    {
        VertexID id{m_nextID++};

        m_vertices.emplace(
            id.value(),
            Vertex{id, point}
        );

        return id;
    }

    EdgeID createEdge()
    {
        EdgeID id{m_nextID++};

        m_edges.emplace(
            id.value(),
            Edge{id}
        );

        return id;
    }

    Vertex* vertex(VertexID id)
    {
        auto it = m_vertices.find(id.value());

        if (it == m_vertices.end())
            return nullptr;

        return &it->second;
    }

    Edge* edge(EdgeID id)
    {
        auto it = m_edges.find(id.value());

        if (it == m_edges.end())
            return nullptr;

        return &it->second;
    }

private:

    std::uint64_t m_nextID = 1;

    std::unordered_map<
        std::uint64_t,
        Vertex
    > m_vertices;

    std::unordered_map<
        std::uint64_t,
        Edge
    > m_edges;
};

}