// MirEngine/Core/Events/EventDispatcher.hpp
// 📡 Диспетчер событий — простой интерфейс для публикации событий в движке.
//
// EventDispatcher — это "дворецкий" событийной системы. Он знает, куда
// отправить событие, и делает это максимально просто. Вместо того чтобы
// каждый модуль хранил ссылку на EventBus и разбирался в типовых индексах,
// все просто вызывают eventDispatcher.publish(myEvent), и диспетчер сам
// находит нужных подписчиков.
//
// Это тонкая обёртка над EventBus, которая:
//   • Принимает владение EventBus при конструировании.
//   • Предоставляет простой метод publish().
//   • Может быть внедрена в любой класс через ссылку/указатель.
//   • Не добавляет накладных расходов — все вызовы делегируются EventBus.
//
// Использование:
//   EventDispatcher dispatcher(bus);
//   dispatcher.publish(event);
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "EventBus.hpp"        // mir::EventBus
#include <memory>              // std::shared_ptr
#include <cassert>             // assert

namespace mir {

class EventDispatcher {
public:
    // ── Конструктор ──────────────────────────────────────────
    // Принимает шину событий (во владение).
    explicit EventDispatcher(std::shared_ptr<EventBus> bus) noexcept
        : m_bus(std::move(bus))
    {
        assert(m_bus != nullptr);   // шина обязательна
    }

    // ── Публикация события ───────────────────────────────────
    // Принимает любое событие (наследник Event) и рассылает его
    // всем подписчикам, зарегистрированным на этот тип события.
    // Возвращает true, если событие было обработано хотя бы одним подписчиком.
    bool publish(Event& event) {
        return m_bus->publish(event);
    }

    // ── Подписка на событие ──────────────────────────────────
    // Предоставляет прямой доступ к подписке (для удобства).
    template<typename T>
    SubscriptionHandle subscribe(std::function<void(T&)> callback) {
        return m_bus->subscribe<T>(std::move(callback));
    }

    // ── Отписка ──────────────────────────────────────────────
    bool unsubscribe(SubscriptionHandle handle) {
        return m_bus->unsubscribe(handle);
    }

    // ── Доступ к шине (если нужен прямой доступ) ─────────────
    [[nodiscard]] EventBus& bus() noexcept { return *m_bus; }
    [[nodiscard]] const EventBus& bus() const noexcept { return *m_bus; }

private:
    std::shared_ptr<EventBus> m_bus;   // владеющая ссылка на шину
};

} // namespace mir