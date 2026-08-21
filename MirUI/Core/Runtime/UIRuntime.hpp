
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

}