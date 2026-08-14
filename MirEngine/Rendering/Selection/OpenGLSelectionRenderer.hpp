#pragma once

#include "SelectionOverlay.hpp"
#include "../OpenGL/OpenGLShaderProgram.hpp"
#include "../Camera/RenderCamera.hpp"

namespace MirEngine {
namespace Rendering {

class OpenGLSelectionRenderer
{
public:
    [[nodiscard]] bool initialize() noexcept;
    void destroy() noexcept;

    void draw(const SelectionOverlay& overlay,
              const RenderMesh& mesh,
              const RenderCamera& camera) const noexcept;

private:
    OpenGLShaderProgram shader_{};
    int viewProjectionLocation_{-1};
};

} // namespace Rendering
} // namespace MirEngine
