// MirUI/Core/Runtime/UIRuntime.hpp
// 🧠 Главный управляющий объект MirUI — единый вход для всего приложения.
//
// UIRuntime владеет всеми ключевыми компонентами ядра и предоставляет
// к ним удобный доступ. Он НЕ зависит от платформы и используется
// как редактором (Designer), так и рендерерами (SwiftUI, WinUI).
//
// В отличие от UIContext, который является просто контейнером данных,
// UIRuntime дополнительно управляет жизненным циклом: инициализацией,
// завершением работы, главным циклом обновления и рендерингом.
//
// Именно UIRuntime будет создаваться при старте приложения и
// передаваться в платформенные адаптеры.
//
// Чистый C++23, без платформенных зависимостей.



#pragma once

#include "../Widget/WidgetTree.hpp"

namespace MirUI {

class UIRuntime {
public:
    bool initialize();
    void shutdown();
    void update(double deltaTime);
    void render(WidgetTree& tree);

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    bool m_initialized = false;
    void renderWidget(Widget* widget);
};

} // namespace MirUI