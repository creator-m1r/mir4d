#pragma once

#include "RayIntersector.hpp"
#include "SelectionOverlay.hpp"

#include <vector>

namespace MirEngine {
namespace Rendering {

class RenderSelectionController
{
public:
    void clear() noexcept
    {
        selected_ = RenderSelection::none();
        hover_ = RenderSelection::none();
        selectedOverlay_.clear();
        hoverOverlay_.clear();
        multiSelected_.clear();
    }

    void hover(const RenderRay& ray, const RenderMesh& mesh) noexcept
    {
        hover_ = RenderSelection::none();
        hoverOverlay_.clear();

        const auto hit = RayIntersector::intersect(ray, mesh);
        if (!hit || hit->triangleIndex >= mesh.triangles.size())
            return;

        const auto faceId = mesh.triangles[hit->triangleIndex].sourceFaceId;
        if (faceId == 0)
            return;

        hover_ = {RenderSelectionType::Face, faceId};
        hoverOverlay_ = SelectionOverlayBuilder::forFace(faceId, mesh);
    }

    void select(const RenderRay& ray,
                const RenderMesh& mesh,
                bool additive = false) noexcept
    {
        const auto hit = RayIntersector::intersect(ray, mesh);
        if (!hit || hit->triangleIndex >= mesh.triangles.size())
        {
            if (!additive)
                clear();
            return;
        }

        const auto faceId = mesh.triangles[hit->triangleIndex].sourceFaceId;
        if (faceId == 0)
            return;

        const RenderSelection selection{RenderSelectionType::Face, faceId};

        if (!additive)
        {
            selected_ = selection;
            selectedOverlay_ = SelectionOverlayBuilder::forFace(faceId, mesh);
            return;
        }

        for (const auto& existing : multiSelected_)
        {
            if (existing.type == selection.type && existing.id == selection.id)
                return;
        }

        multiSelected_.push_back(selection);
    }

    void removeSelection(RenderSelection selection) noexcept;

    [[nodiscard]] const RenderSelection& selected() const noexcept
    {
        return selected_;
    }

    [[nodiscard]] const RenderSelection& hoverSelection() const noexcept
    {
        return hover_;
    }

    [[nodiscard]] const SelectionOverlay& selectedOverlay() const noexcept
    {
        return selectedOverlay_;
    }

    [[nodiscard]] const SelectionOverlay& hoverOverlay() const noexcept
    {
        return hoverOverlay_;
    }

    [[nodiscard]] const std::vector<RenderSelection>& multiSelected() const noexcept
    {
        return multiSelected_;
    }

private:
    RenderSelection selected_{};
    RenderSelection hover_{};
    SelectionOverlay selectedOverlay_{};
    SelectionOverlay hoverOverlay_{};
    std::vector<RenderSelection> multiSelected_{};
};

} // namespace Rendering
} // namespace MirEngine
