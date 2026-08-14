#pragma once

#include <cstdint>
#include <memory>

#include "OpenGLContext.h"
#include "OpenGLDevice.h"
#include "OpenGLShader.h"
#include "../Renderer.h"
#include "../Passes/GridPass.h"
#include "../Passes/GeometryPass.h"
#include "../Resources/ShaderLibrary.h"

namespace MirEngine::Rendering
{

class OpenGLRenderer final : public Renderer
{
public:
    explicit OpenGLRenderer(OpenGLContext* context);
    ~OpenGLRenderer() override;

    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    bool initialize() override;
    void render(mir::Scene& scene, RenderContext& context) override;
    void resize(uint32_t width, uint32_t height) override;

    [[nodiscard]] OpenGLDevice* device() noexcept { return m_device.get(); }
    [[nodiscard]] const OpenGLDevice* device() const noexcept { return m_device.get(); }
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    OpenGLContext* m_context{nullptr};
    std::unique_ptr<OpenGLDevice> m_device;
    ShaderLibrary m_shaderLibrary;
    std::unique_ptr<GridPass> m_gridPass;
    std::unique_ptr<GeometryPass> m_geometryPass;
    bool m_initialized{false};
};

} // namespace MirEngine::Rendering
