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

    void setPlanes(const std::vector<PlaneRenderData>& planes) noexcept
    {
        m_planes = planes;
    }

    void setSketch(const std::vector<SketchRenderData>& sketches) noexcept
    {
        m_sketches = sketches;
    }

    void setCursor(float ndcX, float ndcY, bool active) noexcept
    {
        m_cursorNDC[0] = ndcX;
        m_cursorNDC[1] = ndcY;
        m_cursorActive = active;
    }

    std::string diagnosticsReport();

    void captureDiagnosticFrame(RenderContext& context,
                                mir::Scene& scene,
                                RenderDevice& device);

    [[nodiscard]] OpenGLDevice* device() noexcept { return m_device.get(); }
    [[nodiscard]] const OpenGLDevice* device() const noexcept { return m_device.get(); }
    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

    void setHandSkeletonStyle(const HandSkeletonStyle& style) noexcept override
    {
        if (m_handSkeletonPass)
            m_handSkeletonPass->setStyle(style);
    }

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

}