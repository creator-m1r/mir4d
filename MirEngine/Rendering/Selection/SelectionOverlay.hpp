#pragma once

#include "RenderSelection.hpp"
#include "../Resources/RenderMesh.hpp"

#include <cstdint>
#include <vector>

namespace MirEngine {
namespace Rendering {

struct SelectionOverlay
{
    RenderSelection selection{};
    std::vector<std::uint32_t> triangleIndices;

    [[nodiscard]] bool visible() const noexcept
    {
        return selection.valid() && !triangleIndices.empty();
    }

    void clear() noexcept
    {
        selection = RenderSelection::none();
        triangleIndices.clear();
    }
};

class SelectionOverlayBuilder
{
public:
    [[nodiscard]] static SelectionOverlay forFace(
        std::uint64_t faceId,
        const RenderMesh& mesh) noexcept
    {
        SelectionOverlay result;
        result.selection = {RenderSelectionType::Face, faceId};

        for (std::uint32_t triangleIndex = 0;
             triangleIndex < mesh.triangles.size();
             ++triangleIndex)
        {
            if (mesh.triangles[triangleIndex].sourceFaceId == faceId)
                result.triangleIndices.push_back(triangleIndex);
        }

        if (result.triangleIndices.empty())
            result.clear();

        return result;
    }
};

} // namespace Rendering
} // namespace MirEngine
