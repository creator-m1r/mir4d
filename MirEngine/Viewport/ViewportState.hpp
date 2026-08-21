#pragma once

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Camera.hpp"
#include "ViewportController.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Interaction/PickTypes.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Interaction/SelectionState.hpp"

namespace mir
{

/// A single entry held by the box (multi) selection. Carries the owning
/// object id together with the selected sub-object kind and its mesh-local
/// element id (vertex index, edge id = ti*3 + k, or source B-Rep face id).
struct SelectionEntry
{
    mir4d::ObjectId id{mir4d::InvalidObjectId};
    PickKind kind{PickKind::Body};
    std::uint64_t elementId{0};

    bool operator==(const SelectionEntry& other) const noexcept
    {
        return id == other.id && kind == other.kind && elementId == other.elementId;
    }
};

/// Canonical runtime state owned by a single viewport.
/// Engineering Scene/Document remains the source of truth; this object contains
/// only presentation and interaction state.
struct ViewportState
{
    Camera camera{};
    ViewportController controller{&camera};
    SelectionState selection{};

    // Object id currently under the cursor (hover). Invalid when the cursor
    // is over empty space. Hover never mutates selection.
    mir4d::ObjectId hoveredObjectId{mir4d::InvalidObjectId};
    // Hierarchical kind / element of the hovered sub-object (Body by default).
    PickKind hoveredKind{PickKind::Body};
    std::uint64_t hoveredElementId{0};

    // Hover stabilization bookkeeping (throttle + hysteresis). Pixel positions
    // are in the same bottom-left screen convention as pick().
    Scalar lastHoverPickX{0};
    Scalar lastHoverPickY{0};
    Scalar lastHoverChangeX{0};
    Scalar lastHoverChangeY{0};

    std::uint32_t width{1};
    std::uint32_t height{1};

    /// Active pick filter (selection mode). Defaults to selecting everything.
    PickFilter pickFilter{};

    /// Entries selected via rectangle (box) selection. The primary selection
    /// (SelectionState::selection) mirrors the first of these. When the active
    /// pick filter allows sub-objects, entries carry face/edge/vertex kinds and
    /// their element ids; otherwise they are body-level selections.
    std::vector<SelectionEntry> multiSelection{};

    void resize(std::uint32_t newWidth, std::uint32_t newHeight) noexcept
    {
        width = newWidth == 0 ? 1 : newWidth;
        height = newHeight == 0 ? 1 : newHeight;
        camera.setAspect(static_cast<Scalar>(width) / static_cast<Scalar>(height));
    }

    void setPickFilter(PickFilter filter) noexcept
    {
        pickFilter = filter;
    }

    [[nodiscard]] PickResult pick(const Scene& scene,
                                  Scalar x,
                                  Scalar y) const noexcept
    {
        return RayPicker::pick(scene, camera, x, y, width, height, pickFilter);
    }

    /// Returns the number of objects currently held by the box/multi selection.
    [[nodiscard]] std::size_t multiSelectionCount() const noexcept
    {
        return multiSelection.size();
    }

    /// Returns the object id at the given index in the multi selection, or
    /// InvalidObjectId when the index is out of range.
    [[nodiscard]] mir4d::ObjectId multiSelectionAt(std::size_t index) const noexcept
    {
        if (index >= multiSelection.size())
            return mir4d::InvalidObjectId;
        return multiSelection[index].id;
    }

    /// Returns the selection kind at the given index, or None when out of range.
    [[nodiscard]] PickKind multiSelectionKindAt(std::size_t index) const noexcept
    {
        if (index >= multiSelection.size())
            return PickKind::None;
        return multiSelection[index].kind;
    }

    /// Returns the element id at the given index, or 0 when out of range.
    [[nodiscard]] std::uint64_t multiSelectionElementIdAt(std::size_t index) const noexcept
    {
        if (index >= multiSelection.size())
            return 0;
        return multiSelection[index].elementId;
    }

    void clearMultiSelection() noexcept
    {
        multiSelection.clear();
    }

    /// Selects every object whose projected bounding box intersects the
    /// screen-space rectangle [x0,y0]..[x1,y1] (pixel coords, bottom-left
    /// origin). When `additive` is false the previous multi selection is
    /// replaced; otherwise the new hits are unioned with the existing set.
    /// The primary selection is updated to the first hit (as a body selection).
    void selectInRect(const Scene& scene,
                      Scalar x0,
                      Scalar y0,
                      Scalar x1,
                      Scalar y1,
                      bool additive) noexcept
    {
        const Scalar rx0 = std::min(x0, x1);
        const Scalar ry0 = std::min(y0, y1);
        const Scalar rx1 = std::max(x0, x1);
        const Scalar ry1 = std::max(y0, y1);

        std::vector<mir4d::ObjectId> hits;
        for (const auto& node : scene.nodes())
        {
            if (!node || !node->isValid())
                continue;
            const auto& model = node->model();
            if (!model || model->mesh().empty())
                continue;

            const Transform transform = node->transform();
            const Point3 bmin = transform.transformPoint(model->boundsMin());
            const Point3 bmax = transform.transformPoint(model->boundsMax());

            Scalar pxMin = std::numeric_limits<Scalar>::max();
            Scalar pxMax = std::numeric_limits<Scalar>::lowest();
            Scalar pyMin = std::numeric_limits<Scalar>::max();
            Scalar pyMax = std::numeric_limits<Scalar>::lowest();
            bool anyProjected = false;
            for (int c = 0; c < 8; ++c)
            {
                const Scalar lx = (c & 1) ? bmax.x : bmin.x;
                const Scalar ly = (c & 2) ? bmax.y : bmin.y;
                const Scalar lz = (c & 4) ? bmax.z : bmin.z;
                const auto p = RayPicker::projectToScreen(
                    camera, Point3{lx, ly, lz}, width, height);
                if (!p)
                    continue;
                anyProjected = true;
                pxMin = std::min(pxMin, p->first);
                pxMax = std::max(pxMax, p->first);
                pyMin = std::min(pyMin, p->second);
                pyMax = std::max(pyMax, p->second);
            }
            if (!anyProjected)
                continue;

            const bool intersects =
                pxMax >= rx0 && pxMin <= rx1 && pyMax >= ry0 && pyMin <= ry1;
            if (intersects)
                hits.push_back(node->id());
        }

        // When the body kind is allowed (the default "auto" mode) keep the
        // classic whole-object rectangle selection. Otherwise collect the
        // sub-objects (faces / edges / vertices) whose representative point
        // projects inside the rectangle.
        const bool collectElements = !pickFilter.body;

        std::vector<SelectionEntry> entries;
        if (!collectElements)
        {
            for (const auto id : hits)
                entries.push_back({id, PickKind::Body, 0});
        }
        else
        {
            const PickKind target =
                (!pickFilter.face && !pickFilter.edge && pickFilter.vertex) ? PickKind::Vertex :
                (!pickFilter.face && pickFilter.edge) ? PickKind::Edge :
                pickFilter.face ? PickKind::Face : PickKind::Body;

            for (const auto id : hits)
            {
                const auto node = scene.find(id);
                if (!node || !node->model() || node->model()->mesh().empty())
                    continue;
                const auto& mesh = node->model()->mesh();
                const Transform transform = node->transform();

                if (target == PickKind::Face)
                {
                    std::unordered_map<std::uint64_t, std::array<double, 3>> centroid;
                    std::unordered_map<std::uint64_t, std::size_t> count;
                    for (const auto& tri : mesh.triangles)
                    {
                        const Point3& a = mesh.vertices[tri.a];
                        const Point3& b = mesh.vertices[tri.b];
                        const Point3& c = mesh.vertices[tri.c];
                        auto& s = centroid[tri.sourceFaceId];
                        s[0] += a.x + b.x + c.x;
                        s[1] += a.y + b.y + c.y;
                        s[2] += a.z + b.z + c.z;
                        count[tri.sourceFaceId] += 1;
                    }
                    for (const auto& kv : count)
                    {
                        const auto& s = centroid[kv.first];
                        const Point3 c{s[0] / double(kv.second),
                                       s[1] / double(kv.second),
                                       s[2] / double(kv.second)};
                        const Point3 fc = transform.transformPoint(c);
                        const auto p = RayPicker::projectToScreen(camera, fc, width, height);
                        if (p && p->first >= rx0 && p->first <= rx1 &&
                                p->second >= ry0 && p->second <= ry1)
                            entries.push_back({id, PickKind::Face, kv.first});
                    }
                }
                else if (target == PickKind::Edge)
                {
                    std::unordered_set<std::uint64_t> seen;
                    for (std::size_t ti = 0; ti < mesh.triangles.size(); ++ti)
                    {
                        const auto& tri = mesh.triangles[ti];
                        const std::size_t ends[3][2] = {
                            {tri.a, tri.b}, {tri.b, tri.c}, {tri.c, tri.a}};
                        for (int k = 0; k < 3; ++k)
                        {
                            const std::uint64_t sid = tri.sourceEdgeId[k];
                            if (sid == kInvalidSourceEdge)
                                continue;
                            if (!seen.insert(sid).second)
                                continue;
                            const Point3& a = mesh.vertices[ends[k][0]];
                            const Point3& b = mesh.vertices[ends[k][1]];
                            const Point3 mid{(a.x + b.x) * 0.5,
                                            (a.y + b.y) * 0.5,
                                            (a.z + b.z) * 0.5};
                            const Point3 wp = transform.transformPoint(mid);
                            const auto p = RayPicker::projectToScreen(camera, wp, width, height);
                            if (p && p->first >= rx0 && p->first <= rx1 &&
                                    p->second >= ry0 && p->second <= ry1)
                                entries.push_back(
                                    {id, PickKind::Edge, std::uint64_t(ti * 3 + k)});
                        }
                    }
                }
                else if (target == PickKind::Vertex)
                {
                    std::unordered_set<std::size_t> seen;
                    for (std::size_t vi = 0; vi < mesh.vertices.size(); ++vi)
                    {
                        if (!seen.insert(vi).second)
                            continue;
                        const Point3 wp = transform.transformPoint(mesh.vertices[vi]);
                        const auto p = RayPicker::projectToScreen(camera, wp, width, height);
                        if (p && p->first >= rx0 && p->first <= rx1 &&
                                p->second >= ry0 && p->second <= ry1)
                            entries.push_back({id, PickKind::Vertex, vi});
                    }
                }
            }
        }

        if (!additive)
            multiSelection.clear();
        for (const auto& entry : entries)
        {
            if (std::find(cbegin(multiSelection), cend(multiSelection), entry) == cend(multiSelection))
                multiSelection.push_back(entry);
        }

        if (!multiSelection.empty())
            selection.selectElement(
                multiSelection.front().kind,
                multiSelection.front().id,
                multiSelection.front().elementId,
                false);
        else
            selection.clear();
    }
};

} // namespace mir
