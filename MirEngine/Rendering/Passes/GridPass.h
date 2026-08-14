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
class IndexBuffer;
class OpenGLShader;

class GridPass final : public RenderPass
{
public:
    GridPass();
    ~GridPass() override;

    bool initialize();

    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

    void setGridSize(float size) noexcept { m_gridSize = size; }
    void setMajorStep(float step) noexcept { m_majorStep = step; }
    void setMinorDivisions(int divs) noexcept { m_minorDivisions = divs; }
    void setFadeDistance(float dist) noexcept { m_fadeDistance = dist; }
    void setShowGrid(bool show) noexcept { m_showGrid = show; }
    void setShowAxes(bool show) noexcept { m_showAxes = show; }

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    bool m_initialized{false};
    float m_gridSize{20.0f};
    float m_majorStep{1.0f};
    int m_minorDivisions{10};
    float m_fadeDistance{25.0f};
    bool m_showGrid{true};
    bool m_showAxes{true};

    std::shared_ptr<VertexArray> m_gridVAO;
    std::shared_ptr<VertexBuffer> m_gridVBO;
    uint32_t m_gridVertexCount{0};
    std::shared_ptr<VertexArray> m_axesVAO;
    std::shared_ptr<VertexBuffer> m_axesVBO;
    uint32_t m_axesVertexCount{0};
    std::unique_ptr<OpenGLShader> m_shader;

    void buildGridGeometry();
    void buildAxesGeometry();
    bool createShader();
};

} // namespace MirEngine::Rendering
