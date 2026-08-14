#include "SelectionEventBridge.hpp"

#include "../../MirEngine/Render/Selection/RenderSelection.hpp"

#include <cstdint>

namespace MirUI
{
namespace
{

[[nodiscard]] SelectionKind toSelectionKind(mir::RenderSelectionType type) noexcept
{
    switch (type)
    {
    case mir::RenderSelectionType::Vertex:
        return SelectionKind::Vertex;
    case mir::RenderSelectionType::Edge:
        return SelectionKind::Edge;
    case mir::RenderSelectionType::Face:
        return SelectionKind::Face;
    case mir::RenderSelectionType::Solid:
        return SelectionKind::Solid;
    case mir::RenderSelectionType::None:
    default:
        return SelectionKind::None;
    }
}

} // namespace

void SelectionEventBridge::attach(mir::RenderViewport& viewport) noexcept
{
    viewport.setSelectionChangedCallback(
        [this](const mir::RenderSelection& selection,
               const mir::RenderSelectionProperties&) noexcept
        {
            publish(selection);
        });
}

void SelectionEventBridge::detach(mir::RenderViewport& viewport) noexcept
{
    viewport.setSelectionChangedCallback({});
}

void SelectionEventBridge::publish(const mir::RenderSelection& selection) noexcept
{
    Event event = SelectionChangedEvent::make(
        toSelectionKind(selection.type),
        selection.id);

    eventBus_.publish(event);
}

} // namespace MirUI
