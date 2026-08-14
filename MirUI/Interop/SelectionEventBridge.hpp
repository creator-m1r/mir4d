#pragma once

#include "../Core/Events/EventBus.hpp"
#include "../Core/Events/SelectionChangedEvent.hpp"
#include "../../MirEngine/Render/Viewport/RenderViewport.hpp"

namespace MirUI
{

class SelectionEventBridge
{
public:
    explicit SelectionEventBridge(EventBus& eventBus) noexcept
        : eventBus_(eventBus)
    {
    }

    void attach(mir::RenderViewport& viewport) noexcept;
    void detach(mir::RenderViewport& viewport) noexcept;

private:
    void publish(const mir::RenderSelection& selection) noexcept;

    EventBus& eventBus_;
};

} // namespace MirUI
