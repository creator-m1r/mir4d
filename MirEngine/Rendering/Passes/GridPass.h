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

/// Engineering workspace plane for the grid.
enum class GridPlane
{
    XY = 0, ///< normal +Z (Z-up documents)
    XZ,     ///< ground plane, normal +Y (default, Y-up workspace)
    YZ      ///< normal +X
};

/// Engineering grid overlay.
///
/// The grid is a procedural infinite grid rendered as a single full-screen
/// triangle. The fragment shader reconstructs the world-space point on the
/// ground plane for every pixel and draws anti-aliased minor/major lines plus
/// the two in-plane axes. No per-frame vertex buffer uploads are performed, so
/// the pass is cheap and free of GL buffer churn (the previous CPU-quad
/// builder re-uploaded dynamic buffers every frame).
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
    GridPlane m_plane{GridPlane::XZ};
    float m_fadeDistanceOverride{0.0f};
    bool m_showGrid{true};
    bool m_showAxes{true};

    std::unique_ptr<OpenGLShader> m_gridShader;
    std::unique_ptr<OpenGLShader> m_bgShader;

    // Static full-screen geometry (built once in initialize).
    std::shared_ptr<VertexBuffer> m_gridVBO;
    std::shared_ptr<IndexBuffer> m_gridIBO;
    std::shared_ptr<VertexArray> m_gridVAO;

    std::shared_ptr<VertexBuffer> m_bgVBO;
    std::shared_ptr<IndexBuffer> m_bgIBO;
    std::shared_ptr<VertexArray> m_bgVAO;

    bool createShaders();
    void buildBackground(RenderDevice& device);
    void buildGridQuad(RenderDevice& device);

    [[nodiscard]] static double niceStep(double target) noexcept;
    static void planeAxes(GridPlane plane,
                          float normal[3],
                          float point[3],
                          float uDir[3],
                          float vDir[3]) noexcept;
};

} // namespace MirEngine::Rendering
