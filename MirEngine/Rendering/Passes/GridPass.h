#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/Vertex.h"

#include <cstdint>
#include <memory>

namespace MirEngine::Rendering
{

class Shader;
class VertexArray;
class VertexBuffer;
class OpenGLShader;

/// Grid plane selection (engineering workspace plane).
enum class GridPlane
{
    XY = 0, ///< ground plane, normal +Z
    XZ,     ///< normal +Y
    YZ      ///< normal +X
};

/// Procedural infinite grid pass.
///
/// The grid is not a geometry object: a full-screen quad is shaded with a
/// ray -> plane intersection computed per-pixel. The step adapts to the
/// camera scale (1/2/5 x 10^n), major lines are emphasized every 5 minor
/// steps, and the plane fades smoothly at the far distance. All grid math
/// is camera-relative (anchor snapped in double precision on the CPU), so
/// huge world coordinates never reach the GPU as giant floats.
class GridPass final : public RenderPass
{
public:
    GridPass();
    ~GridPass() override;

    bool initialize();

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

    std::shared_ptr<VertexArray> m_quadVAO;
    std::shared_ptr<VertexBuffer> m_quadVBO;
    std::shared_ptr<VertexArray> m_axisVAO;
    std::shared_ptr<VertexBuffer> m_axisVBO;
    std::unique_ptr<OpenGLShader> m_gridShader;
    std::unique_ptr<OpenGLShader> m_axisShader;

    bool createShaders();
    void buildQuad();
    void buildAxis();

    /// 1/2/5 x 10^n rounding of a target step.
    [[nodiscard]] static double niceStep(double target) noexcept;
    static void planeBasis(GridPlane plane,
                                         float normal[3],
                                         float origin[3],
                                         float axisAColor[3],
                                         float axisBColor[3],
                                         float verticalColor[3]) noexcept;
};

} // namespace MirEngine::Rendering