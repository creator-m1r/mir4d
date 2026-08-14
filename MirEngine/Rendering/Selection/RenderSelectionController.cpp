#include "RenderSelectionController.hpp"

namespace MirEngine {
namespace Rendering {

void RenderSelectionController::removeSelection(RenderSelection selection) noexcept
{
    if (!selection.valid())
        return;

    for (auto it = multiSelected_.begin(); it != multiSelected_.end(); ++it)
    {
        if (it->type == selection.type && it->id == selection.id)
        {
            multiSelected_.erase(it);
            return;
        }
    }
}

} // namespace Rendering
} // namespace MirEngine
