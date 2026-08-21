
#pragma once

#include "../Widget/WidgetTree.hpp"

namespace MirUI {

class Renderer {
public:

    virtual ~Renderer() = default;

    virtual void beginFrame() {}

    virtual void render(WidgetTree& tree) = 0;

    virtual void endFrame() {}
};

}