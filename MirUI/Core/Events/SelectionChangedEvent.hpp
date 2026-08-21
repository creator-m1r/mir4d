#pragma once

#include "Event.hpp"

#include <cstdint>

namespace MirUI
{

enum class SelectionKind : std::uint64_t
{
    None = 0,
    Vertex = 1,
    Edge = 2,
    Face = 3,
    Solid = 4,
    Object = 5
};

struct SelectionChangedEvent : Event
{
    SelectionKind selectionKind{SelectionKind::None};
    std::uint64_t selectionId{0};

    SelectionChangedEvent() noexcept
    {
        type = EventType::SelectionChanged;
    }

    SelectionChangedEvent(SelectionKind kind,
                          std::uint64_t id,
                          WidgetID target = {}) noexcept
        : SelectionChangedEvent()
    {
        selectionKind = kind;
        selectionId = id;
        this->target = target;
        data0 = static_cast<std::uint64_t>(kind);
        data1 = id;
    }

    [[nodiscard]] static Event make(SelectionKind kind,
                                    std::uint64_t id,
                                    WidgetID target = {}) noexcept
    {
        return SelectionChangedEvent(kind, id, target);
    }
};

} // namespace MirUI
