#pragma once

// MirEngine/BRep/Topology/BRepTopologyEditor.hpp
// Единственная точка изменения межсущностных топологических связей.
// Геометрические данные редактор не создаёт и не дублирует.

#include "MirEngine/BRep/Topology/BRepTopologyStore.hpp"

#include <algorithm>

namespace mir
{

class BRepTopologyEditor
{
public:
    [[nodiscard]] static bool addEdgeToWire(
        BRepTopologyStore& topology,
        BRepWireHandle wireHandle,
        BRepOrientedEdge orientedEdge)
    {
        auto* wire = topology.wire(wireHandle);
        if (!wire || !orientedEdge.valid() || !topology.edge(orientedEdge.edge))
            return false;

        if (std::find(wire->edges.begin(), wire->edges.end(), orientedEdge) != wire->edges.end())
            return true;

        // A closed wire is immutable until explicitly rebuilt.
        if (wire->closed)
            return false;

        wire->edges.push_back(orientedEdge);
        wire->closed = wireIsClosed(topology, *wire);
        wire->free = false;
        return true;
    }

    [[nodiscard]] static bool setOuterWire(
        BRepTopologyStore& topology,
        BRepFaceHandle faceHandle,
        BRepOrientedWire wire)
    {
        auto* face = topology.face(faceHandle);
        auto* wireRecord = topology.wire(wire.wire);
        if (!face || !wire.valid() || !wireRecord || !wireRecord->closed)
            return false;

        if (wireRecord->ownerFace.valid() && wireRecord->ownerFace != faceHandle)
            return false;

        face->outer = wire;
        face->free = false;
        wireRecord->ownerFace = faceHandle;
        wireRecord->free = false;
        return true;
    }

    [[nodiscard]] static bool addInnerWire(
        BRepTopologyStore& topology,
        BRepFaceHandle faceHandle,
        BRepOrientedWire wire)
    {
        auto* face = topology.face(faceHandle);
        auto* wireRecord = topology.wire(wire.wire);
        if (!face || !wire.valid() || !wireRecord || !wireRecord->closed)
            return false;

        if (wireRecord->ownerFace.valid() && wireRecord->ownerFace != faceHandle)
            return false;

        if (std::find(face->inners.begin(), face->inners.end(), wire) != face->inners.end())
            return true;

        face->inners.push_back(wire);
        face->free = false;
        wireRecord->ownerFace = faceHandle;
        wireRecord->free = false;
        return true;
    }

    [[nodiscard]] static bool addFaceToShell(
        BRepTopologyStore& topology,
        BRepShellHandle shellHandle,
        BRepOrientedFace face)
    {
        auto* shell = topology.shell(shellHandle);
        auto* faceRecord = topology.face(face.face);
        if (!shell || !face.valid() || !faceRecord || !faceRecord->outer.valid())
            return false;

        if (faceRecord->ownerShell.valid() && faceRecord->ownerShell != shellHandle)
            return false;

        if (std::find(shell->faces.begin(), shell->faces.end(), face) != shell->faces.end())
            return true;

        shell->faces.push_back(face);
        shell->closed = false;
        shell->free = false;
        faceRecord->ownerShell = shellHandle;
        faceRecord->free = false;
        return true;
    }

    [[nodiscard]] static bool addShellToSolid(
        BRepTopologyStore& topology,
        BRepSolidHandle solidHandle,
        BRepShellHandle shellHandle)
    {
        auto* solid = topology.solid(solidHandle);
        auto* shell = topology.shell(shellHandle);
        if (!solid || !shell || !shellHandle.valid())
            return false;

        if (shell->ownerSolid.valid() && shell->ownerSolid != solidHandle)
            return false;

        if (std::find(solid->shells.begin(), solid->shells.end(), shellHandle) != solid->shells.end())
            return true;

        solid->shells.push_back(shellHandle);
        solid->free = false;
        shell->ownerSolid = solidHandle;
        shell->free = false;
        return true;
    }

    [[nodiscard]] static bool markShellClosed(
        BRepTopologyStore& topology,
        BRepShellHandle shellHandle,
        bool closed)
    {
        auto* shell = topology.shell(shellHandle);
        if (!shell || shell->faces.empty())
            return false;

        if (closed && !shellBoundariesClosed(topology, *shell))
            return false;

        shell->closed = closed;
        return true;
    }

    [[nodiscard]] static bool markWireClosed(
        BRepTopologyStore& topology,
        BRepWireHandle wireHandle,
        bool closed)
    {
        auto* wire = topology.wire(wireHandle);
        if (!wire || wire->edges.empty())
            return false;

        if (closed && !wireIsClosed(topology, *wire))
            return false;

        wire->closed = closed;
        return true;
    }

private:
    [[nodiscard]] static BRepVertexHandle orientedStart(
        const BRepTopologyStore& topology,
        const BRepOrientedEdge& item) noexcept
    {
        const auto* edge = topology.edge(item.edge);
        if (!edge)
            return {};
        return isForward(item.orientation) ? edge->start : edge->end;
    }

    [[nodiscard]] static BRepVertexHandle orientedEnd(
        const BRepTopologyStore& topology,
        const BRepOrientedEdge& item) noexcept
    {
        const auto* edge = topology.edge(item.edge);
        if (!edge)
            return {};
        return isForward(item.orientation) ? edge->end : edge->start;
    }

    [[nodiscard]] static bool wireIsClosed(
        const BRepTopologyStore& topology,
        const BRepWire& wire) noexcept
    {
        if (wire.edges.empty())
            return false;

        for (std::size_t i = 1; i < wire.edges.size(); ++i)
        {
            if (orientedEnd(topology, wire.edges[i - 1]) !=
                orientedStart(topology, wire.edges[i]))
                return false;
        }

        const auto first = orientedStart(topology, wire.edges.front());
        const auto last = orientedEnd(topology, wire.edges.back());
        return first.valid() && last.valid() && first == last;
    }

    [[nodiscard]] static bool shellBoundariesClosed(
        const BRepTopologyStore& topology,
        const BRepShell& shell) noexcept
    {
        for (const BRepOrientedFace& orientedFace : shell.faces)
        {
            const auto* face = topology.face(orientedFace.face);
            if (!face)
                return false;

            const auto* outer = topology.wire(face->outer.wire);
            if (!outer || !wireIsClosed(topology, *outer))
                return false;

            for (const BRepOrientedWire& inner : face->inners)
            {
                const auto* wire = topology.wire(inner.wire);
                if (!wire || !wireIsClosed(topology, *wire))
                    return false;
            }
        }
        return true;
    }
};

} // namespace mir
