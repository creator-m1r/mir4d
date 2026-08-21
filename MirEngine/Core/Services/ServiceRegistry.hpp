
#pragma once

#include "IService.hpp"
#include "../Result.hpp"
#include <memory>
#include <vector>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <stdexcept>

namespace mir {

using mir4d::Result;
using mir4d::ErrorCode;

class ServiceRegistry {
public:

    ServiceRegistry() noexcept = default;
    
    ~ServiceRegistry() {
        if (m_initialized) {
            shutdownAll();
        }
    }

    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    template<typename T, typename... Args>
    T& registerService(Args&&... args) {

        if (hasService<T>()) {
            throw std::runtime_error(
                "ServiceRegistry::registerService: сервис типа " +
                std::string(typeid(T).name()) + " уже зарегистрирован."
            );
        }

        if (m_initialized) {
            throw std::runtime_error(
                "ServiceRegistry::registerService: нельзя регистрировать сервисы после инициализации."
            );
        }

        auto service = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *service;
        m_services.push_back(std::move(service));
        return ref;
    }

    template<typename T>
    [[nodiscard]] T& get() {
        static_assert(std::is_base_of_v<IService, T>,
                      "T должен наследовать IService");

        for (auto& service : m_services) {

            T* ptr = dynamic_cast<T*>(service.get());
            if (ptr) {

                if (!ptr->isReady()) {
                    throw std::runtime_error(
                        "ServiceRegistry::get: сервис " + ptr->name() + " не готов (isReady() == false)."
                    );
                }
                return *ptr;
            }
        }

        throw std::runtime_error(
            "ServiceRegistry::get: сервис типа " +
            std::string(typeid(T).name()) + " не зарегистрирован."
        );
    }

    template<typename T>
    [[nodiscard]] const T& get() const {

        return const_cast<ServiceRegistry*>(this)->get<T>();
    }

    template<typename T>
    [[nodiscard]] bool hasService() const noexcept {
        for (const auto& service : m_services) {
            if (dynamic_cast<const T*>(service.get())) {
                return true;
            }
        }
        return false;
    }

    Result<void> initializeAll() {
        if (m_initialized) {
            return std::unexpected(
                mir4d::Error{ErrorCode::InvalidState,
                             "ServiceRegistry already initialized"});
        }

        for (auto& service : m_services) {
            auto result = service->initialize();
            if (!result) {

                return result;
            }
        }

        m_initialized = true;
        return {};
    }

    void shutdownAll() {

        for (auto it = m_services.rbegin(); it != m_services.rend(); ++it) {
            if ((*it)->isReady()) {
                (*it)->shutdown();
            }
        }
        m_initialized = false;
    }

    [[nodiscard]] bool isInitialized() const noexcept {
        return m_initialized;
    }

    [[nodiscard]] size_t count() const noexcept {
        return m_services.size();
    }

private:

    std::vector<std::unique_ptr<IService>> m_services;

    bool m_initialized = false;
};

}