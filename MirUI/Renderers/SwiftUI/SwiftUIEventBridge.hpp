// MirUI/Renderers/SwiftUI/SwiftUIEventBridge.hpp
// 🍎 Конкретная реализация моста событий для macOS.
//
// SwiftUIEventBridge преобразует нативные события macOS (NSEvent)
// в универсальные события MirUI (MirUI::Event) и передаёт их
// в EventDispatcher ядра. Также он может получать события из ядра
// и выполнять действия на платформе (например, закрыть окно).
//
// Используется внутри SwiftUI-адаптера для связи между
// нативным интерфейсом и C++ ядром.
//
// Чистый C++23 внутри, но требует Objective-C++ для работы с NSEvent.
// Этот заголовок (.hpp) не содержит платформенных типов,
// поэтому его можно подключать из обычного C++ кода.
// Реализация (.mm) будет написана на Objective-C++.

#pragma once

#include "../../Core/Bridge/UIEventBridge.hpp"
#include "../../Core/Events/Event.hpp"
#include "../../Core/Widget/WidgetID.hpp"
#include <memory>

namespace MirUI {

// Вперёд объявляем платформенный тип, чтобы не подключать AppKit в заголовке.
// Реальный тип будет использован только в .mm файле.
#ifdef __OBJC__
@class NSEvent;
#else
struct NSEvent; // заглушка для C++ кода
#endif

class SwiftUIEventBridge : public UIEventBridge {
public:
    SwiftUIEventBridge();
    virtual ~SwiftUIEventBridge();

    // ── Преобразование NSEvent в MirUI::Event ────────────────
    // Этот метод вызывается из Swift/Objective-C кода, когда
    // происходит нативное событие (клик, движение мыши, клавиша…).
    // Он анализирует тип NSEvent и заполняет поля MirUI::Event.
    // Возвращает true, если событие было успешно преобразовано
    // и отправлено в ядро.
    bool handleNSEvent(const NSEvent* nsEvent, WidgetID targetWidget);

    // ── Реализация виртуального метода createEvent ───────────
    // Создаёт пустое событие (заглушка). Реальная работа
    // происходит в handleNSEvent.
    Event createEvent() override;

    // ── Отправка события из ядра в платформу ─────────────────
    // Вызывается ядром, когда нужно выполнить действие на платформе
    // (например, закрыть окно, показать диалог).
    void dispatchPlatformEvent(const Event& event);

private:
    // Внутренние данные (реализация в .mm)
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    // Преобразование типа NSEventType в MirUI::EventType
    static EventType nsEventTypeToMirUI(long nsType);

    // Извлечение координат мыши из NSEvent
    static void extractMouseCoordinates(const NSEvent* nsEvent, double& x, double& y);

    // Извлечение информации о клавише из NSEvent
    static void extractKeyInfo(const NSEvent* nsEvent, int& keyCode, bool& ctrl, bool& shift, bool& alt, bool& cmd);
};

} // namespace MirUI