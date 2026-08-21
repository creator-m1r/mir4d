#pragma once

#include "Event.hpp"
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mir4d {

using SubscriptionHandle = std::uint64_t;

class EventBus {
public:
    EventBus() noexcept = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    template <typename T, typename F>
    SubscriptionHandle subscribe(F&& callback) {
        static_assert(std::is_base_of_v<Event, T>,
                      "Event type must derive from mir4d::Event");

        const SubscriptionHandle handle =
            m_nextHandle.fetch_add(1, std::memory_order_relaxed);

        auto wrapper = [cb = std::forward<F>(callback)](Event& event) mutable {
            cb(static_cast<T&>(event));
        };

        m_subscribers[typeid(T)].push_back(
            Subscriber{handle, std::move(wrapper)});
        return handle;
    }

    bool unsubscribe(SubscriptionHandle handle) noexcept {
        for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it) {
            auto& subscribers = it->second;
            const auto subscriber = std::find_if(
                subscribers.begin(), subscribers.end(),
                [handle](const Subscriber& value) {
                    return value.handle == handle;
                });

            if (subscriber == subscribers.end()) {
                continue;
            }

            subscribers.erase(subscriber);
            if (subscribers.empty()) {
                m_subscribers.erase(it);
            }
            return true;
        }
        return false;
    }

    bool publish(Event& event) {
        const auto it = m_subscribers.find(typeid(event));
        if (it == m_subscribers.end()) {
            return false;
        }

        bool delivered = false;

        const auto subscribers = it->second;
        for (const auto& subscriber : subscribers) {
            subscriber.callback(event);
            delivered = true;
            if (event.isHandled()) {
                break;
            }
        }
        return delivered;
    }

    void clear() noexcept {
        m_subscribers.clear();
    }

private:
    struct Subscriber {
        SubscriptionHandle handle;
        std::function<void(Event&)> callback;
    };

    std::unordered_map<std::type_index, std::vector<Subscriber>> m_subscribers;
    std::atomic<SubscriptionHandle> m_nextHandle{1};
};

}
