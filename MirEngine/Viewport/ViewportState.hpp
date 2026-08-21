#pragma once

#include <algorithm>
#include <limits>
#include <vector>

#include "Camera.hpp"
#include "ViewportController.hpp"
#include "MirEngine/Interaction/RayPicker.hpp"
#include "MirEngine/Interaction/SelectionState.hpp"

namespace mir
{

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

    /// Set of objects selected via rectangle (box) selection. The primary
    /// selection (SelectionState::selection) mirrors the first of these.
    std::vector<mir4d::ObjectId> multiSelection{};

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
        return multiSelection[index];
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

        if (!additive)
            multiSelection.clear();
        for (const auto id : hits)
        {
            if (std::find(cbegin(multiSelection), cend(multiSelection), id) == cend(multiSelection))
                multiSelection.push_back(id);
        }

        if (!multiSelection.empty())
            selection.selectElement(PickKind::Body, multiSelection.front(), 0, false);
        else
            selection.clear();
    }
};

} // namespace mir
