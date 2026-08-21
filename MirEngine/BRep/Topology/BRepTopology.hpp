#pragma once

#include "MirEngine/BRep/Topology/BRepTypes.hpp"
#include "MirEngine/BRep/Core/BRepHandles.hpp"
#include "MirEngine/BRep/Geometry/BRepGeometry.hpp"

#include <vector>

namespace mir
{

struct BRepVertex
{
    BRepVertexHandle self{};
    BRepPointHandle point{};
    double tolerance{DefaultBRepTolerance.linear};
    bool free{true};

    [[nodiscard]] bool isValid() const noexcept
    {
        return self.valid() && point.valid() && tolerance > 0.0;
    }
};

struct BRepEdge
{
    BRepEdgeHandle self{};
    BRepCurveHandle curve{};
    BRepRange range{};
    BRepVertexHandle start{};
    BRepVertexHandle end{};
    double tolerance{DefaultBRepTolerance.linear};
    bool degenerated{false};
    bool sameParameter{true};
    bool sameRange{true};
    bool free{true};

    [[nodiscard]] bool isClosed() const noexcept
    {
        return start.valid() && end.valid() && start == end;
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!self.valid() || tolerance <= 0.0)
            return false;
        if (degenerated)
            return start.valid();
        return curve.valid() &&
               range.isValid() &&
               start.valid() &&
               end.valid();
    }
};

struct BRepWire
{
    BRepWireHandle self{};
    std::vector<BRepOrientedEdge> edges;
    BRepFaceHandle ownerFace{};
    bool closed{false};
    bool free{true};

    [[nodiscard]] bool empty() const noexcept
    {
        return edges.empty();
    }

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!self.valid())
            return false;
        for (const BRepOrientedEdge& item : edges)
        {
            if (!item.valid())
                return false;
        }
        return true;
    }
};

struct BRepFace
{
    BRepFaceHandle self{};
    BRepSurfaceHandle surface{};
    BRepOrientation orientation{BRepOrientation::Forward};
    BRepOrientedWire outer{};
    std::vector<BRepOrientedWire> inners;
    BRepShellHandle ownerShell{};
    double tolerance{DefaultBRepTolerance.linear};
    bool naturalRestriction{false};
    bool free{true};

    [[nodiscard]] bool isValid() const noexcept
    {
        return self.valid() &&
               surface.valid() &&
               outer.valid() &&
               tolerance > 0.0;
    }
};

struct BRepShell
{
    BRepShellHandle self{};
    std::vector<BRepOrientedFace> faces;
    BRepSolidHandle ownerSolid{};
    bool closed{false};
    bool free{true};

    [[nodiscard]] bool isValid() const noexcept
    {
        if (!self.valid())
            return false;
        for (const BRepOrientedFace& item : faces)
        {
            if (!item.valid())
                return false;
        }
        return !faces.empty();
    }
};

struct BRepSolid
{
    BRepSolidHandle self{};
    std::vector<BRepShellHandle> shells;
    bool free{true};

    [[nodiscard]] bool isValid() const noexcept
    {
        return self.valid() && !shells.empty();
    }
};

}
