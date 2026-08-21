#pragma once

#include "MirEngine/BRep/Topology/BRepTopology.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace mir
{

class BRepTopologyStore
{
public:
    struct Checkpoint
    {
        std::size_t vertexCount{0};
        std::size_t edgeCount{0};
        std::size_t wireCount{0};
        std::size_t faceCount{0};
        std::size_t shellCount{0};
        std::size_t solidCount{0};
    };

    [[nodiscard]] Checkpoint checkpoint() const noexcept
    {
        return {
            vertices_.size(), edges_.size(), wires_.size(),
            faces_.size(), shells_.size(), solids_.size()
        };
    }

    void rollback(Checkpoint checkpoint) noexcept
    {
        if (checkpoint.vertexCount <= vertices_.size()) vertices_.resize(checkpoint.vertexCount);
        if (checkpoint.edgeCount <= edges_.size()) edges_.resize(checkpoint.edgeCount);
        if (checkpoint.wireCount <= wires_.size()) wires_.resize(checkpoint.wireCount);
        if (checkpoint.faceCount <= faces_.size()) faces_.resize(checkpoint.faceCount);
        if (checkpoint.shellCount <= shells_.size()) shells_.resize(checkpoint.shellCount);
        if (checkpoint.solidCount <= solids_.size()) solids_.resize(checkpoint.solidCount);
    }

    [[nodiscard]] BRepVertexHandle addVertex(BRepVertex vertex)
    {
        const BRepIndex index = static_cast<BRepIndex>(vertices_.size());
        vertex.self = BRepVertexHandle{index};
        vertices_.push_back(std::move(vertex));
        return vertices_.back().self;
    }

    [[nodiscard]] BRepEdgeHandle addEdge(BRepEdge edge)
    {
        const BRepIndex index = static_cast<BRepIndex>(edges_.size());
        edge.self = BRepEdgeHandle{index};
        edges_.push_back(std::move(edge));
        return edges_.back().self;
    }

    [[nodiscard]] BRepWireHandle addWire(BRepWire wire)
    {
        const BRepIndex index = static_cast<BRepIndex>(wires_.size());
        wire.self = BRepWireHandle{index};
        wires_.push_back(std::move(wire));
        return wires_.back().self;
    }

    [[nodiscard]] BRepFaceHandle addFace(BRepFace face)
    {
        const BRepIndex index = static_cast<BRepIndex>(faces_.size());
        face.self = BRepFaceHandle{index};
        faces_.push_back(std::move(face));
        return faces_.back().self;
    }

    [[nodiscard]] BRepShellHandle addShell(BRepShell shell)
    {
        const BRepIndex index = static_cast<BRepIndex>(shells_.size());
        shell.self = BRepShellHandle{index};
        shells_.push_back(std::move(shell));
        return shells_.back().self;
    }

    [[nodiscard]] BRepSolidHandle addSolid(BRepSolid solid)
    {
        const BRepIndex index = static_cast<BRepIndex>(solids_.size());
        solid.self = BRepSolidHandle{index};
        solids_.push_back(std::move(solid));
        return solids_.back().self;
    }

    [[nodiscard]] const BRepVertex* vertex(BRepVertexHandle handle) const noexcept { return get(vertices_, handle.index); }
    [[nodiscard]] BRepVertex* vertex(BRepVertexHandle handle) noexcept { return get(vertices_, handle.index); }
    [[nodiscard]] const BRepEdge* edge(BRepEdgeHandle handle) const noexcept { return get(edges_, handle.index); }
    [[nodiscard]] BRepEdge* edge(BRepEdgeHandle handle) noexcept { return get(edges_, handle.index); }
    [[nodiscard]] const BRepWire* wire(BRepWireHandle handle) const noexcept { return get(wires_, handle.index); }
    [[nodiscard]] BRepWire* wire(BRepWireHandle handle) noexcept { return get(wires_, handle.index); }
    [[nodiscard]] const BRepFace* face(BRepFaceHandle handle) const noexcept { return get(faces_, handle.index); }
    [[nodiscard]] BRepFace* face(BRepFaceHandle handle) noexcept { return get(faces_, handle.index); }
    [[nodiscard]] const BRepShell* shell(BRepShellHandle handle) const noexcept { return get(shells_, handle.index); }
    [[nodiscard]] BRepShell* shell(BRepShellHandle handle) noexcept { return get(shells_, handle.index); }
    [[nodiscard]] const BRepSolid* solid(BRepSolidHandle handle) const noexcept { return get(solids_, handle.index); }
    [[nodiscard]] BRepSolid* solid(BRepSolidHandle handle) noexcept { return get(solids_, handle.index); }

    [[nodiscard]] const std::vector<BRepVertex>& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const std::vector<BRepEdge>& edges() const noexcept { return edges_; }
    [[nodiscard]] const std::vector<BRepWire>& wires() const noexcept { return wires_; }
    [[nodiscard]] const std::vector<BRepFace>& faces() const noexcept { return faces_; }
    [[nodiscard]] const std::vector<BRepShell>& shells() const noexcept { return shells_; }
    [[nodiscard]] const std::vector<BRepSolid>& solids() const noexcept { return solids_; }

    [[nodiscard]] std::size_t vertexCount() const noexcept { return vertices_.size(); }
    [[nodiscard]] std::size_t edgeCount() const noexcept { return edges_.size(); }
    [[nodiscard]] std::size_t wireCount() const noexcept { return wires_.size(); }
    [[nodiscard]] std::size_t faceCount() const noexcept { return faces_.size(); }
    [[nodiscard]] std::size_t shellCount() const noexcept { return shells_.size(); }
    [[nodiscard]] std::size_t solidCount() const noexcept { return solids_.size(); }

    void clear() noexcept
    {
        vertices_.clear();
        edges_.clear();
        wires_.clear();
        faces_.clear();
        shells_.clear();
        solids_.clear();
    }

private:
    template <typename T>
    [[nodiscard]] static const T* get(const std::vector<T>& items, BRepIndex index) noexcept
    {
        if (!isValidBRepIndex(index) || index >= items.size()) return nullptr;
        return &items[index];
    }

    template <typename T>
    [[nodiscard]] static T* get(std::vector<T>& items, BRepIndex index) noexcept
    {
        if (!isValidBRepIndex(index) || index >= items.size()) return nullptr;
        return &items[index];
    }

    std::vector<BRepVertex> vertices_;
    std::vector<BRepEdge> edges_;
    std::vector<BRepWire> wires_;
    std::vector<BRepFace> faces_;
    std::vector<BRepShell> shells_;
    std::vector<BRepSolid> solids_;
};

}