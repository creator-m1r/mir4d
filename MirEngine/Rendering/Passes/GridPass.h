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

/// Grid plane selection (engineering workspace plane).
enum class GridPlane
{
    XY = 0, ///< ground plane, normal +Z
    XZ,     ///< normal +Y
    YZ      ///< normal +X
};

/// Procedural engineering grid pass.
///
/// The grid is generated on the CPU every frame as screen-aligned line quads:
///   - line positions are computed in double precision and shifted by the
///     camera position, so GPU floats stay small (camera-relative rendering,
///     same contract as GeometryPass);
///   - the step adapts to the camera (1/2/5 x 10^n) with a target of ~18 px
///     per cell; major lines are emphasized every 5 steps;
///   - every line quad is ~1-2 px wide in screen space with a soft alpha edge,
///     so lines stay smooth and 1 px crisp without MSAA;
///   - per-vertex fade removes lines smoothly at the far distance;
///   - the workspace axes (X red, Y green, Z blue) are drawn from the world
///     origin; the studio gradient background is an opaque full-screen layer.
///
/// All state changes are routed through RenderDevice.
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
    static constexpr std::uint32_t kQuadIndexCount = 6;
    static constexpr double kExtendFactor = 1.18;   // overscan for line fade
    static constexpr double kMaxLinesPerAxis = 220.0;

    bool m_initialized{false};
    GridPlane m_plane{GridPlane::XY};
    float m_fadeDistanceOverride{0.0f};
    bool m_showGrid{true};
    bool m_showAxes{true};

    std::unique_ptr<OpenGLShader> m_lineShader;
    std::unique_ptr<OpenGLShader> m_bgShader;

    std::shared_ptr<VertexBuffer> m_gridVBO;
    std::shared_ptr<IndexBuffer> m_gridIBO;
    std::shared_ptr<VertexArray> m_gridVAO;

    std::shared_ptr<VertexBuffer> m_bgVBO;
    std::shared_ptr<IndexBuffer> m_bgIBO;
    std::shared_ptr<VertexArray> m_bgVAO;

    bool createShaders();
    void buildBackground(RenderDevice& device);
    void rebuildLines(RenderContext& context, RenderDevice& device);

    /// 1/2/5 x 10^n rounding of a target step.
    [[nodiscard]] static double niceStep(double target) noexcept;
    static void planeBasis(GridPlane plane,
                           float normal[3],
                           float uDir[3],
                           float vDir[3],
                           double anchor[3],
                           const double eye[3],
                           double majorStep) noexcept;
};

} // namespace MirEngine::Rendering