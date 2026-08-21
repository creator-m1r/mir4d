#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "OpenGLContext.h"
#include "OpenGLDevice.h"
#include "OpenGLShader.h"
#include "../Renderer.h"
#include "../Core/RenderContext.h"
#include "../Passes/GridPass.h"
#include "../Passes/GeometryPass.h"
#include "../Passes/PlanePass.h"
#include "../Passes/SketchPass.h"
#include "../Passes/HandSkeletonPass.h"
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

    /// Sets the work planes overlaid in the viewport (ТЗ Этап 1). The renderer
    /// copies them into the per-frame context so PlanePass can draw them.
    void setPlanes(const std::vector<PlaneRenderData>& planes) noexcept
    {
        m_planes = planes;
    }

    /// Sets the 2D sketch overlay drawn on a work plane (ТЗ Этап 2). The
    /// renderer copies it into the per-frame context so SketchPass can draw it.
    void setSketch(const std::vector<SketchRenderData>& sketches) noexcept
    {
        m_sketches = sketches;
    }

    /// Sets the cursor position in normalized device coordinates for plane
    /// hover picking. active=false clears the hover when the pointer leaves.
    void setCursor(float ndcX, float ndcY, bool active) noexcept
    {
        m_cursorNDC[0] = ndcX;
        m_cursorNDC[1] = ndcY;
        m_cursorActive = active;
    }

    /// Text report about the OpenGL context (requires the current context;
    /// performs makeCurrent itself).
    std::string diagnosticsReport();

    /// TEMP DIAGNOSTIC: renders one frame into an offscreen FBO and writes a
    /// PPM so the output can be inspected headlessly (MIR4D_SCREENSHOT=1).
    void captureDiagnosticFrame(RenderContext& context,
                                mir::Scene& scene,
                                RenderDevice& device);

    [[nodiscard]] OpenGLDevice* device() noexcept { return m_device.get(); }
    [[nodiscard]] const OpenGLDevice* device() const noexcept { return m_device.get(); }
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

    /// Forwards the hand-skeleton overlay style (colours / sizes / depth) to the
    /// dedicated pass. Called once (or when the configuration changes).
    void setHandSkeletonStyle(const HandSkeletonStyle& style) noexcept override
    {
        if (m_handSkeletonPass)
            m_handSkeletonPass->setStyle(style);
    }

    /// Forwards the hand-skeleton bone topology (single source of truth comes
    /// from Swift MIRHandSkeletonTopology).
    void setHandSkeletonTopology(const std::vector<std::pair<int, int>>& bones) noexcept override
    {
        if (m_handSkeletonPass)
            m_handSkeletonPass->setTopology(bones);
    }

private:
    OpenGLContext* m_context{nullptr};
    std::unique_ptr<OpenGLDevice> m_device;
    ShaderLibrary m_shaderLibrary;
    std::unique_ptr<GridPass> m_gridPass;
    std::unique_ptr<GeometryPass> m_geometryPass;
    std::unique_ptr<PlanePass> m_planePass;
    std::unique_ptr<SketchPass> m_sketchPass;
    std::unique_ptr<HandSkeletonPass> m_handSkeletonPass;
    std::vector<PlaneRenderData> m_planes;
    std::vector<SketchRenderData> m_sketches;
    float m_cursorNDC[2]{0.0f, 0.0f};
    bool m_cursorActive{false};
    bool m_initialized{false};
};

} // namespace MirEngine::Rendering