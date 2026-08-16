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
/// The grid is generated on the CPU every frame as screen-sized line quads
/// (camera-relative, double precision, same contract as GeometryPass). The
/// shader receives u_view and u_projection as two separate uniforms - exactly
/// like GeometryPass - so no manual matrix multiplication is involved.
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
    static constexpr double kExtendFactor = 1.18;   // overscan for line fade
    static constexpr double kMaxLinesPerAxis = 220.0;

    bool m_initialized{false};
    GridPlane m_plane{GridPlane::XZ};
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

    [[nodiscard]] static double niceStep(double target) noexcept;
    static void planeBasis(GridPlane plane,
                           float normal[3],
                           float uDir[3],
                           float vDir[3],
                           double anchor[3],
                           const double eye[3],
                           double step) noexcept;
};

} // namespace MirEngine::Rendering
