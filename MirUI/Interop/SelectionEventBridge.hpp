#pragma once

#include "../Core/Events/EventBus.hpp"
#include "../Core/Events/SelectionChangedEvent.hpp"
#include "../../MirEngine/Rendering/Viewport/RenderViewport.hpp"

namespace MirUI
{

class SelectionEventBridge
{
public:
    explicit SelectionEventBridge(EventBus& eventBus) noexcept
        : eventBus_(eventBus)
    {
    }

    void attach(MirEngine::Rendering::RenderViewport& viewport) noexcept;
    void detach(MirEngine::Rendering::RenderViewport& viewport) noexcept;

private:
    void publish(const MirEngine::Rendering::RenderSelection& selection) noexcept;

    EventBus& eventBus_;
};

} // namespace MirUI
