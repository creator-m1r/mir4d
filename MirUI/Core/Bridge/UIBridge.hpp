// MirUI/Core/Bridge/UIBridge.hpp
// 🌉 Базовый мост между MirUI Core и платформенным рендерером.
//
// UIBridge определяет интерфейс, который должен реализовать каждый
// платформенный адаптер (SwiftUI, WinUI, WebUI).
// Ядро MirUI вызывает методы этого интерфейса, чтобы:
//   • Передать актуальный снимок дерева виджетов (present).
//   • Запросить перерисовку (update).
//   • Получить события от платформы (dispatchEvent).
//
// Благодаря этому абстрактному классу ядро не зависит от конкретной
// платформы — оно работает с любым адаптером одинаково.
//
// Чистый C++23, без платформенных зависимостей.

#pragma once

#include "../Widget/WidgetTreeSnapshot.hpp"
#include "../Events/Event.hpp"        // базовое событие MirUI
#include <memory>

namespace MirUI {

class UIBridge {
public:
    virtual ~UIBridge() = default;

    // ── Передать снимок интерфейса платформенному рендереру ──
    // Вызывается ядром после каждого изменения дерева виджетов.
    // Адаптер должен построить из снимка нативные View и отобразить их.
    virtual void present(const WidgetTreeSnapshot& snapshot) = 0;

    // ── Запросить перерисовку ────────────────────────────────
    // Вызывается ядром, когда нужно обновить экран (например,
    // после изменения темы или завершения анимации).
    virtual void update() = 0;

    // ── Отправить событие от платформы в ядро ────────────────
    // Вызывается адаптером, когда пользователь взаимодействует
    // с интерфейсом (клик, ввод текста, изменение размера окна…).
    // Событие передаётся в EventDispatcher ядра.
    virtual void dispatchEvent(const Event& event) = 0;

    // ── Получить последний снимок (если нужно адаптеру) ──────
    // Может возвращать nullptr, если present() ещё не вызывался.
    [[nodiscard]] virtual const WidgetTreeSnapshot* lastSnapshot() const {
        return m_lastSnapshot.get();
    }

protected:
    // Адаптер может сохранить последний снимок для внутренних нужд.
    void setLastSnapshot(std::unique_ptr<WidgetTreeSnapshot> snapshot) {
        m_lastSnapshot = std::move(snapshot);
    }

private:
    std::unique_ptr<WidgetTreeSnapshot> m_lastSnapshot;
};

} // namespace MirUI