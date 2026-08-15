#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "OpenGLContext.h"
#include "OpenGLDevice.h"
#include "OpenGLShader.h"
#include "../Renderer.h"
#include "../Passes/GridPass.h"
#include "../Passes/GeometryPass.h"
#include "../Resources/ShaderLibrary.h"

namespace MirEngine::Rendering
{

/// OpenGL implementation of the Renderer contract.
///
/// Owns the device, the shader library and the pass pipeline:
///
///   background (GridPass) -> grid/axes (GridPass) -> scene geometry
///   (GeometryPass, camera-relative with selection highlight)
///
/// The class does not own the OpenGLContext; the C ABI layer (MirEngineExports)
/// owns the context and the renderer lifetime.
class OpenGLRenderer final : public Renderer
{
public:
    explicit OpenGLRenderer(OpenGLContext* context);
    ~OpenGLRenderer() override;

    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    bool initialize() override;
    void render(mir::Scene& scene, RenderContext& context) override;
    void resize(std::uint32_t width, std::uint32_t height) override;
    void setObjectMaterial(std::uint64_t objectId,
                           std::int32_t materialId) noexcept override;

    void setGridPlane(GridPlane plane) noexcept { if (m_gridPass) m_gridPass->setPlane(plane); }

    /// Text report about the OpenGL context (requires the current context;
    /// performs makeCurrent itself).
    std::string diagnosticsReport();

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