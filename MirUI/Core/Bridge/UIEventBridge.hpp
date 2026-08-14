// MirUI/Core/Bridge/UIEventBridge.hpp
// 📡 Мост событий — передача платформенных взаимодействий в ядро MirUI.
//
// Когда пользователь нажимает кнопку, двигает мышь или вводит текст,
// платформенный адаптер (SwiftUI, WinUI) получает системное событие.
// UIEventBridge преобразует его в универсальное MirUI::Event и доставляет
// в EventDispatcher ядра. Тот, в свою очередь, рассылает событие нужным
// виджетам, которые могут вызвать команды или изменить состояние.
//
// Это абстрактный класс — конкретная реализация зависит от платформы.
// Например, SwiftUIEventBridge знает, как из NSEvent сделать MirUI::Event,
// а WinUIEventBridge работает с Windows::UI::Core::PointerEventArgs.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Events/Event.hpp"            // MirUI::Event, EventType
#include "../Widget/WidgetID.hpp"         // WidgetID
#include "../Widget/WidgetTree.hpp"       // WidgetTree — необходим для диспетчера
#include "../Events/EventDispatcher.hpp"  // EventDispatcher
#include <functional>
#include <memory>

namespace MirUI {

class UIEventBridge {
public:
    virtual ~UIEventBridge() = default;

    // ── Подключение к диспетчеру событий ядра ────────────────
    // Вызывается при инициализации. Все события, полученные от платформы,
    // будут передаваться в этот диспетчер.
    void setEventDispatcher(EventDispatcher* dispatcher) {
        m_dispatcher = dispatcher;
    }

    // ── Установка ссылки на дерево виджетов (нужен для диспетчера) ──
    void setWidgetTree(WidgetTree* tree) {
        m_widgetTree = tree;
    }

    // ── Отправить событие в ядро ─────────────────────────────
    // Вызывается адаптером, когда происходит платформенное событие.
    // Событие уже преобразовано в универсальный MirUI::Event.
    // Создаётся изменяемая копия, потому что EventDispatcher::dispatch
    // принимает неконстантную ссылку и может модифицировать событие
    // (например, устанавливать флаг handled).
    virtual void dispatchEvent(const Event& event) {
        if (m_dispatcher && m_widgetTree) {
            Event mutableEvent = event;                 // создаём неконстантную копию
            m_dispatcher->dispatch(*m_widgetTree, mutableEvent); // передаём в ядро
        }
    }

    // ── Создать событие из платформенных данных ──────────────
    // Чисто виртуальный метод — каждая платформа реализует по-своему.
    // Принимает сырые данные (например, координаты мыши, код клавиши)
    // и возвращает готовый MirUI::Event.
    virtual Event createEvent() = 0;

    // ── Колбэк для обработки событий из ядра ──────────────────
    // Если ядро хочет отправить событие обратно в платформу
    // (например, запросить закрытие окна), оно вызывает этот колбэк.
    using OutgoingEventCallback = std::function<void(const Event&)>;
    void setOutgoingEventCallback(OutgoingEventCallback callback) {
        m_outgoingCallback = std::move(callback);
    }

protected:
    EventDispatcher* m_dispatcher = nullptr;
    WidgetTree*      m_widgetTree = nullptr;   // нужен для вызова dispatch
    OutgoingEventCallback m_outgoingCallback;

    // Отправить событие из ядра в платформу.
    void sendToPlatform(const Event& event) {
        if (m_outgoingCallback) {
            m_outgoingCallback(event);
        }
    }
};

} // namespace MirUI