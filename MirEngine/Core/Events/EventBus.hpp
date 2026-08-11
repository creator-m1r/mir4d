// MirEngine/Core/Events/EventBus.hpp
// 📡 Шина событий — центральная система обмена сообщениями в MirEngine.
//
// В сложном движке десятки модулей должны общаться друг с другом:
//   • Документ изменился → нужно обновить дерево проекта и 3D-вид.
//   • Пользователь выделил объект → нужно показать его свойства в инспекторе.
//   • Завершилась операция моделирования → нужно обновить BRep и перерисовать сцену.
//
// Если модули будут вызывать друг друга напрямую, получится запутанная паутина
// зависимостей. EventBus решает эту проблему через паттерн «издатель-подписчик»:
//   • Любой модуль может опубликовать событие (publish).
//   • Любой модуль может подписаться на определённый тип события (subscribe).
//   • Издатель не знает, кто подписан, а подписчик не знает, кто издатель.
//   • Все общаются только через EventBus и базовый класс Event.
//
// EventBus работает с типами событий (через typeid). При публикации события
// он находит всех подписчиков, заинтересованных в этом конкретном типе,
// и вызывает их колбэки. Если подписчик устанавливает флаг handled = true,
// EventBus останавливает дальнейшую рассылку (событие «съедено»).
//
// Использование:
//   EventBus bus;
//   bus.subscribe<EntityCreated>([](EntityCreated& e) {
//       Logger::info("Создана сущность {}", e.entityId);
//   });
//   EntityCreated event(entityId);
//   bus.publish(event);
//
// Чистый C++23, без внешних зависимостей.

#pragma once

#include "Event.hpp"                 // Базовый класс Event
#include <unordered_map>              // Хранение подписчиков по типам
#include <vector>                     // Список подписчиков на один тип
#include <functional>                 // std::function для колбэков
#include <typeindex>                  // std::type_index для идентификации типов
#include <atomic>                     // std::atomic для генерации handle'ов
#include <cstdint>                    // uint64_t
#include <algorithm>                  // std::find_if

namespace mir {

using SubscriptionHandle = uint64_t;

class EventBus {
public:
    EventBus() noexcept = default;

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    template<typename T>
    SubscriptionHandle subscribe(std::function<void(T&)> callback) {
        static_assert(std::is_base_of_v<Event, T>,
                      "T должен наследоваться от Event");

        auto wrapper = [cb = std::move(callback)](Event& event) {
            T& concrete = static_cast<T&>(event);
            cb(concrete);
        };

        SubscriptionHandle handle = m_nextHandle.fetch_add(1, std::memory_order_relaxed);
        std::type_index typeIdx = std::type_index(typeid(T));
        m_subscribers[typeIdx].push_back({handle, std::move(wrapper)});
        return handle;
    }

    bool unsubscribe(SubscriptionHandle handle) {
        for (auto& [typeIdx, subscribers] : m_subscribers) {
            auto it = std::find_if(subscribers.begin(), subscribers.end(),
                [handle](const Subscriber& sub) { return sub.handle == handle; });
            if (it != subscribers.end()) {
                subscribers.erase(it);
                if (subscribers.empty()) {
                    m_subscribers.erase(typeIdx);
                }
                return true;
            }
        }
        return false;
    }

    bool publish(Event& event) {
        std::type_index typeIdx = std::type_index(typeid(event));
        auto it = m_subscribers.find(typeIdx);
        if (it == m_subscribers.end()) {
            return false;
        }

        bool handled = false;
        for (auto& subscriber : it->second) {
            subscriber.callback(event);
            handled = true;
            if (event.isHandled()) {
                break;
            }
        }
        return handled;
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

} // namespace mir