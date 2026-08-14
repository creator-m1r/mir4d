// MirEngine/Core/Services/ServiceRegistry.hpp
// 📋 Реестр сервисов — управляет жизненным циклом всех сервисов MirEngine.
//
// В MirEngine вся логика разделена на независимые сервисы: GeometryService,
// DocumentService, SceneService, SelectionService и так далее. Каждый сервис
// реализует интерфейс IService (инициализация, завершение, проверка готовности).
//
// ServiceRegistry — это центральный "дирижёр" для всех сервисов. Он:
//   • Хранит все зарегистрированные сервисы.
//   • Инициализирует их в порядке регистрации.
//   • Завершает в обратном порядке (LIFO — последний зарегистрированный,
//     первый выключенный). Это важно, потому что сервисы могут зависеть
//     друг от друга: сцена зависит от геометрии, рендеринг зависит от сцены.
//     Завершать нужно в обратном порядке зависимостей.
//   • Позволяет найти сервис по его типу (через шаблонный метод get<T>()).
//   • Гарантирует, что сервис не будет использоваться до инициализации
//     и после завершения.
//
// Реестр сам по себе НЕ наследует IService — он управляет другими сервисами,
// а не является одним из них. Это разделение ответственности: реестр —
// менеджер, сервисы — работники.
//
// Использование:
//   ServiceRegistry registry;
//   registry.registerService<GeometryService>();
//   registry.registerService<SceneService>();
//   registry.initializeAll();                    // инициализирует все сервисы
//   auto& geom = registry.get<GeometryService>(); // получаем сервис
//   registry.shutdownAll();                      // корректно завершает все
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "IService.hpp"                // IService — базовый интерфейс сервиса
#include "../Result.hpp"               // mir4d::Result, mir4d::ErrorCode
#include <memory>                      // std::unique_ptr
#include <vector>                      // хранение списка сервисов
#include <string>                      // для сообщений об ошибках
#include <typeinfo>                    // typeid для проверки типа
#include <algorithm>                   // std::find_if
#include <stdexcept>                   // std::runtime_error

namespace mir {

using mir4d::Result;
using mir4d::ErrorCode;

class ServiceRegistry {
public:
    // ── Конструктор и деструктор ─────────────────────────────
    ServiceRegistry() noexcept = default;
    
    // Деструктор автоматически завершает все сервисы, если они ещё работают.
    ~ServiceRegistry() {
        if (m_initialized) {
            shutdownAll();
        }
    }

    // Запрет копирования — реестр владеет уникальными сервисами.
    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    // ── Регистрация сервиса ──────────────────────────────────
    // Создаёт сервис типа T (должен наследовать IService) и сохраняет его
    // в реестре. Сервис ещё НЕ инициализирован — это произойдёт при вызове
    // initializeAll(). Регистрировать сервисы можно только ДО инициализации.
    //
    // Шаблонный параметр T — конкретный класс сервиса (например, GeometryService).
    // Аргументы args... передаются в конструктор T.
    template<typename T, typename... Args>
    T& registerService(Args&&... args) {
        // Проверяем, что сервис ещё не был зарегистрирован.
        if (hasService<T>()) {
            throw std::runtime_error(
                "ServiceRegistry::registerService: сервис типа " +
                std::string(typeid(T).name()) + " уже зарегистрирован."
            );
        }

        // Проверяем, что инициализация ещё не началась.
        if (m_initialized) {
            throw std::runtime_error(
                "ServiceRegistry::registerService: нельзя регистрировать сервисы после инициализации."
            );
        }

        // Создаём сервис и сохраняем в списке.
        auto service = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *service;
        m_services.push_back(std::move(service));
        return ref;
    }

    // ── Поиск сервиса по типу ────────────────────────────────
    // Возвращает ссылку на сервис типа T. Если сервис не найден,
    // выбрасывает исключение. Используй только после успешной инициализации.
    //
    // Пример:
    //   auto& geom = registry.get<GeometryService>();
    //   geom.createBox(100, 50, 30);
    template<typename T>
    [[nodiscard]] T& get() {
        static_assert(std::is_base_of_v<IService, T>,
                      "T должен наследовать IService");

        for (auto& service : m_services) {
            // Пытаемся привести указатель к T*. Если не получилось — dynamic_cast вернёт nullptr.
            T* ptr = dynamic_cast<T*>(service.get());
            if (ptr) {
                // Проверяем, что сервис готов к использованию.
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

    // Константная версия поиска.
    template<typename T>
    [[nodiscard]] const T& get() const {
        // Снимаем константность для вызова неконстантной версии.
        return const_cast<ServiceRegistry*>(this)->get<T>();
    }

    // ── Проверка наличия сервиса ─────────────────────────────
    // Возвращает true, если сервис типа T был зарегистрирован.
    template<typename T>
    [[nodiscard]] bool hasService() const noexcept {
        for (const auto& service : m_services) {
            if (dynamic_cast<const T*>(service.get())) {
                return true;
            }
        }
        return false;
    }

    // ── Инициализация всех сервисов ──────────────────────────
    // Инициализирует все зарегистрированные сервисы в порядке их регистрации.
    // Если какой-либо сервис не смог инициализироваться, останавливается
    // и возвращает ошибку. Уже инициализированные сервисы остаются в памяти
    // (их можно выключить через shutdownAll()).
    Result<void> initializeAll() {
        if (m_initialized) {
            return std::unexpected(
                mir4d::Error{ErrorCode::InvalidState,
                             "ServiceRegistry already initialized"});
        }

        for (auto& service : m_services) {
            auto result = service->initialize();
            if (!result) {
                // Ошибка при инициализации одного из сервисов.
                // Не откатываем остальные — вызывающий может попробовать
                // снова или вызвать shutdownAll() для очистки.
                return result;
            }
        }

        m_initialized = true;
        return {};   // успех
    }

    // ── Завершение всех сервисов ─────────────────────────────
    // Выключает все сервисы в порядке, ОБРАТНОМ регистрации.
    // Это гарантирует, что сервисы, зависящие от других, будут выключены
    // раньше, чем их зависимости. Например, SceneService зависит от
    // GeometryService, поэтому SceneService нужно выключить первым.
    void shutdownAll() {
        // Идём с конца к началу — обратный порядок.
        for (auto it = m_services.rbegin(); it != m_services.rend(); ++it) {
            if ((*it)->isReady()) {
                (*it)->shutdown();
            }
        }
        m_initialized = false;
    }

    // ── Проверка состояния инициализации ──────────────────────
    [[nodiscard]] bool isInitialized() const noexcept {
        return m_initialized;
    }

    // ── Количество зарегистрированных сервисов ────────────────
    [[nodiscard]] size_t count() const noexcept {
        return m_services.size();
    }

private:
    // Список всех зарегистрированных сервисов в порядке регистрации.
    // Каждый элемент — уникальный владелец своего сервиса.
    std::vector<std::unique_ptr<IService>> m_services;

    // Флаг: были ли сервисы уже инициализированы?
    bool m_initialized = false;
};

} // namespace mir