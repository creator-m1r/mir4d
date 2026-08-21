#pragma once

#include "RenderPass.h"

#include <cstdint>
#include <memory>

namespace MirEngine::Rendering
{

class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class RenderDevice;
class OpenGLShader;

enum class GridPlane
{
    XY = 0,
    XZ,
    YZ
};

class GridPass final : public RenderPass
{
public:
    GridPass();
    ~GridPass() override;

    bool initialize(RenderDevice& device);

    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

    void setPlane(GridPlane plane) noexcept { m_plane = plane; }
    [[nodiscard]] GridPlane plane() const noexcept { return m_plane; }

    void setFadeDistance(float dist) noexcept { m_fadeDistanceOverride = dist; }
    void setShowGrid(bool show) noexcept { m_showGrid = show; }
    void setShowAxes(bool show) noexcept { m_showAxes = show; }

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    bool m_initialized{false};
    GridPlane m_plane{GridPlane::XY};
    float m_fadeDistanceOverride{0.0f};
    bool m_showGrid{true};
    bool m_showAxes{true};

    std::unique_ptr<OpenGLShader> m_gridShader;
    std::unique_ptr<OpenGLShader> m_bgShader;
    std::unique_ptr<OpenGLShader> m_axisShader;

    std::shared_ptr<VertexBuffer> m_gridVBO;
    std::shared_ptr<IndexBuffer> m_gridIBO;
    std::shared_ptr<VertexArray> m_gridVAO;

    std::shared_ptr<VertexBuffer> m_bgVBO;
    std::shared_ptr<IndexBuffer> m_bgIBO;
    std::shared_ptr<VertexArray> m_bgVAO;

    std::shared_ptr<VertexBuffer> m_axisVBO;
    std::shared_ptr<VertexArray> m_axisVAO;

    bool createShaders();
    void buildBackground(RenderDevice& device);
    void buildGridQuad(RenderDevice& device);
    void buildAxisGizmo(RenderDevice& device);

    [[nodiscard]] static double niceStep(double target) noexcept;
    static void planeAxes(GridPlane plane,
                          float normal[3],
                          float point[3],
                          float uDir[3],
                          float vDir[3]) noexcept;
};

}
