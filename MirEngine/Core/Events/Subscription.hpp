#pragma once

#include "EventBus.hpp"
#include <utility>

namespace mir4d {

class Subscription {
public:
    Subscription() noexcept = default;

    Subscription(EventBus* bus, SubscriptionHandle handle) noexcept
        : m_bus(bus), m_handle(handle) {
        if (handle == 0) {
            m_bus = nullptr;
        }
    }

    ~Subscription() { reset(); }

    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    Subscription(Subscription&& other) noexcept
        : m_bus(std::exchange(other.m_bus, nullptr)),
          m_handle(std::exchange(other.m_handle, 0)) {}

    Subscription& operator=(Subscription&& other) noexcept {
        if (this != &other) {
            reset();
            m_bus = std::exchange(other.m_bus, nullptr);
            m_handle = std::exchange(other.m_handle, 0);
        }
        return *this;
    }

    void reset() noexcept {
        if (m_bus != nullptr && m_handle != 0) {
            m_bus->unsubscribe(m_handle);
        }
        m_bus = nullptr;
        m_handle = 0;
    }

    [[nodiscard]] bool valid() const noexcept {
        return m_bus != nullptr && m_handle != 0;
    }

    [[nodiscard]] explicit operator bool() const noexcept { return valid(); }
    [[nodiscard]] SubscriptionHandle handle() const noexcept { return m_handle; }

private:
    EventBus* m_bus{nullptr};
    SubscriptionHandle m_handle{0};
};

}
