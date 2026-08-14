#pragma once

#include "OpenGLMeshRenderer.hpp"
#include "OpenGLShader.hpp"
#include "../RenderCamera.hpp"

namespace mir
{

class OpenGLFrameRenderer
{
public:
    [[nodiscard]] bool initialize() noexcept;
    void destroy() noexcept;

    void resize(int width, int height) noexcept;
    void beginFrame() const noexcept;
    void draw(const RenderMeshGPU& mesh, const RenderCamera& camera) noexcept;
    void endFrame() const noexcept;

    [[nodiscard]] bool valid() const noexcept { return initialized_; }

private:
    bool initialized_{false};
    OpenGLShader shader_{};
    OpenGLMeshRenderer meshRenderer_{};
    int width_{1};
    int height_{1};
    int viewProjectionLocation_{-1};
};

} // namespace mir
