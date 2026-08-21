#pragma once

#include "Event.hpp"
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MirUI
{

class EventBus
{
public:
    using Handler = std::function<void(Event&)>;

    void subscribe(EventType type, Handler handler)
    {
        handlers_[type].push_back(std::move(handler));
    }

    void publish(Event& event)
    {
        const auto it = handlers_.find(event.type);
        if (it == handlers_.end())
            return;

        for (auto& handler : it->second)
        {
            handler(event);
            if (event.handled)
                break;
        }
    }

    void clear(EventType type)
    {
        handlers_.erase(type);
    }

    void clear() noexcept
    {
        handlers_.clear();
    }

private:
    std::unordered_map<EventType, std::vector<Handler>> handlers_{};
};

}
