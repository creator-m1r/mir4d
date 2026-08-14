#pragma once

#include "SelectionOverlay.hpp"
#include "../OpenGL/OpenGLShader.hpp"
#include "../RenderCamera.hpp"

namespace mir
{

class OpenGLSelectionRenderer
{
public:
    [[nodiscard]] bool initialize() noexcept;
    void destroy() noexcept;

    void draw(const SelectionOverlay& overlay,
              const RenderMesh& mesh,
              const RenderCamera& camera) const noexcept;

private:
    OpenGLShader shader_{};
    int viewProjectionLocation_{-1};
};

} // namespace mir
