// MirEngine/Core/Events/Subscription.hpp
// 🎫 Умная подписка на события — автоматически отписывается при разрушении.
//
// Когда ты подписываешься на событие в EventBus, тебе возвращается
// SubscriptionHandle (число). По этому числу ты должен потом отписаться,
// иначе память и ресурсы не освободятся, а подписчик продолжит получать
// события даже после того, как объект-подписчик уже удалён.
//
// Subscription — это "умная" обёртка над SubscriptionHandle, которая
// автоматически вызывает unsubscribe в деструкторе. Пока объект Subscription
// живёт, подписка активна. Как только он удаляется (например, при выходе
// из метода или при разрушении родительского класса), подписка снимается
// автоматически. Это очень удобно и безопасно: не надо помнить про ручную
// отписку.
//
// Subscription нельзя копировать (иначе одна подписка отписалась бы дважды),
// но можно перемещать. При перемещении старый объект теряет подписку
// (становится "пустым"), а новый — забирает её себе.
//
// Использование:
//   Subscription sub = bus.subscribe<EntityCreated>([](EntityCreated& e) { ... });
//   // подписка активна...
//   // когда sub выйдет из области видимости, автоматически отпишется.
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "EventBus.hpp"          // EventBus, SubscriptionHandle
#include <functional>            // std::function (неявно через EventBus)
#include <cassert>               // assert
#include <utility>               // std::exchange, std::move

namespace mir {

class Subscription {
public:
    // ── Конструктор по умолчанию (пустая подписка) ────────────
    // Создаёт неактивную подписку. Никакой handle не хранится,
    // EventBus не вызывается. Используй, когда подписка не обязательна.
    Subscription() noexcept = default;

    // ── Основной конструктор ─────────────────────────────────
    // Принимает указатель на EventBus и дескриптор подписки.
    // Запоминает их и будет использовать для автоматической отписки.
    Subscription(EventBus* bus, SubscriptionHandle handle) noexcept
        : m_bus(bus)
        , m_handle(handle)
    {
        // Если передали невалидный handle, делаем подписку пустой.
        if (handle == 0) {
            m_bus = nullptr;
        }
    }

    // ── Деструктор: автоматическая отписка ────────────────────
    // Если подписка активна, вызываем unsubscribe.
    ~Subscription() {
        reset();
    }

    // ── Запрет копирования ───────────────────────────────────
    // Нельзя скопировать подписку — иначе возникнет двойная отписка.
    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    // ── Разрешение перемещения ────────────────────────────────
    // При перемещении источник теряет подписку (становится пустым).
    Subscription(Subscription&& other) noexcept
        : m_bus(std::exchange(other.m_bus, nullptr))
        , m_handle(std::exchange(other.m_handle, 0))
    {}

    Subscription& operator=(Subscription&& other) noexcept {
        if (this != &other) {
            reset();                           // сначала отписываем свою
            m_bus    = std::exchange(other.m_bus, nullptr);
            m_handle = std::exchange(other.m_handle, 0);
        }
        return *this;
    }

    // ── Ручная отписка ───────────────────────────────────────
    // Вызывает unsubscribe и очищает подписку. Можно вызывать несколько раз.
    void reset() noexcept {
        if (m_bus && m_handle != 0) {
            m_bus->unsubscribe(m_handle);
        }
        m_bus    = nullptr;
        m_handle = 0;
    }

    // ── Проверка валидности ──────────────────────────────────
    [[nodiscard]] bool valid() const noexcept {
        return m_bus != nullptr && m_handle != 0;
    }

    // ── Явное приведение к bool ───────────────────────────────
    // Позволяет писать: if (sub) { ... }
    [[nodiscard]] explicit operator bool() const noexcept {
        return valid();
    }

    // ── Получение дескриптора (только для чтения) ─────────────
    [[nodiscard]] SubscriptionHandle handle() const noexcept {
        return m_handle;
    }

private:
    EventBus*          m_bus    = nullptr;   // указатель на шину (не владеет)
    SubscriptionHandle m_handle = 0;         // дескриптор подписки
};

} // namespace mir