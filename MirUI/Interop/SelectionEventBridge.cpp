#include "SelectionEventBridge.hpp"

#include "../../MirEngine/Rendering/Selection/RenderSelection.hpp"

#include <cstdint>

namespace MirUI
{
namespace
{

[[nodiscard]] SelectionKind toSelectionKind(MirEngine::Rendering::RenderSelectionType type) noexcept
{
    switch (type)
    {
    case MirEngine::Rendering::RenderSelectionType::Vertex:
        return SelectionKind::Vertex;
    case MirEngine::Rendering::RenderSelectionType::Edge:
        return SelectionKind::Edge;
    case MirEngine::Rendering::RenderSelectionType::Face:
        return SelectionKind::Face;
    case MirEngine::Rendering::RenderSelectionType::Solid:
        return SelectionKind::Solid;
    case MirEngine::Rendering::RenderSelectionType::None:
    default:
        return SelectionKind::None;
    }
}

} // namespace

void SelectionEventBridge::attach(MirEngine::Rendering::RenderViewport& viewport) noexcept
{
    viewport.setSelectionChangedCallback(
        [this](const MirEngine::Rendering::RenderSelection& selection,
               const MirEngine::Rendering::RenderSelectionProperties&) noexcept
        {
            publish(selection);
        });
}

void SelectionEventBridge::detach(MirEngine::Rendering::RenderViewport& viewport) noexcept
{
    viewport.setSelectionChangedCallback({});
}

void SelectionEventBridge::publish(const MirEngine::Rendering::RenderSelection& selection) noexcept
{
    Event event = SelectionChangedEvent::make(
        toSelectionKind(selection.type),
        selection.id);

    eventBus_.publish(event);
}

} // namespace MirUI
