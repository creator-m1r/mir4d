#pragma once

#include "SketchExtrudeGeometry.hpp"
#include "SketchExtrudeSectionBuilder.hpp"
#include "SketchExtrudeSurfaceBuilder.hpp"

#include <cstdint>
#include <vector>

namespace mir
{

enum class SketchExtrudeBoundaryFaceKind : std::uint8_t
{
    Bottom,
    Top,
    Side
};

struct SketchExtrudeBoundaryEdge
{
    std::uint32_t geometryID{0};
    std::uint32_t sectionIndex{0};
    bool reversed{false};
};

struct SketchExtrudeBoundaryFace
{
    SketchExtrudeBoundaryFaceKind kind{SketchExtrudeBoundaryFaceKind::Side};
    std::vector<SketchExtrudeBoundaryEdge> edges;
    std::uint32_t sourceGeometryID{0};
};

struct SketchExtrudeBoundary
{
    std::vector<SketchExtrudeBoundaryFace> faces;
    bool valid{false};

    [[nodiscard]] bool isClosed() const noexcept
    {
        return valid && !faces.empty();
    }
};

class SketchExtrudeBoundaryBuilder
{
public:
    [[nodiscard]] static SketchExtrudeBoundary build(
        const SketchExtrudeSections& sections,
        const SketchExtrudeSurfaceSet& surfaces) noexcept
    {
        SketchExtrudeBoundary boundary;

        if (sections.bottom.curves.empty() || sections.top.curves.empty() ||
            surfaces.sideSurfaces.empty())
            return boundary;

        boundary.faces.push_back(makeSectionFace(
            SketchExtrudeBoundaryFaceKind::Bottom,
            sections.bottom,
            true));

        boundary.faces.push_back(makeSectionFace(
            SketchExtrudeBoundaryFaceKind::Top,
            sections.top,
            false));

        for (const auto& surface : surfaces.sideSurfaces)
        {
            SketchExtrudeBoundaryFace face;
            face.kind = SketchExtrudeBoundaryFaceKind::Side;
            face.sourceGeometryID = surface.sourceGeometryID;
            face.edges.push_back({surface.sourceGeometryID, 0, false});
            boundary.faces.push_back(std::move(face));
        }

        boundary.valid = boundary.faces.size() >= 3;
        return boundary;
    }

private:
    [[nodiscard]] static SketchExtrudeBoundaryFace makeSectionFace(
        SketchExtrudeBoundaryFaceKind kind,
        const SketchExtrudeSection& section,
        bool reversed) noexcept
    {
        SketchExtrudeBoundaryFace face;
        face.kind = kind;

        for (std::uint32_t i = 0; i < section.curves.size(); ++i)
        {
            face.edges.push_back({section.curves[i].geometryID, i, reversed});
        }
        return face;
    }
};

}
