#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace mir4d {

/// Explicit owner of long-lived MirEngine services.
/// Services are registered by concrete type; no global singleton is required.
class ServiceContainer {
public:
    ServiceContainer() = default;
    ~ServiceContainer() = default;

    ServiceContainer(const ServiceContainer&) = delete;
    ServiceContainer& operator=(const ServiceContainer&) = delete;
    ServiceContainer(ServiceContainer&&) noexcept = default;
    ServiceContainer& operator=(ServiceContainer&&) noexcept = default;

    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        const auto key = std::type_index(typeid(T));
        auto service = std::make_shared<T>(std::forward<Args>(args)...);
        T& result = *service;
        m_services.insert_or_assign(key, std::move(service));
        return result;
    }

    template <typename T>
    [[nodiscard]] T* get() noexcept {
        const auto it = m_services.find(std::type_index(typeid(T)));
        if (it == m_services.end()) {
            return nullptr;
        }
        return static_cast<T*>(it->second.get());
    }

    template <typename T>
    [[nodiscard]] const T* get() const noexcept {
        const auto it = m_services.find(std::type_index(typeid(T)));
        if (it == m_services.end()) {
            return nullptr;
        }
        return static_cast<const T*>(it->second.get());
    }

    template <typename T>
    T& require() {
        if (auto* service = get<T>(); service != nullptr) {
            return *service;
        }
        throw std::logic_error("mir4d service is not registered");
    }

    template <typename T>
    const T& require() const {
        if (const auto* service = get<T>(); service != nullptr) {
            return *service;
        }
        throw std::logic_error("mir4d service is not registered");
    }

    template <typename T>
    bool contains() const noexcept {
        return m_services.contains(std::type_index(typeid(T)));
    }

    template <typename T>
    bool remove() noexcept {
        return m_services.erase(std::type_index(typeid(T))) != 0;
    }

    void clear() noexcept { m_services.clear(); }

    [[nodiscard]] bool empty() const noexcept { return m_services.empty(); }
    [[nodiscard]] std::size_t size() const noexcept { return m_services.size(); }

private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_services;
};

} // namespace mir4d
