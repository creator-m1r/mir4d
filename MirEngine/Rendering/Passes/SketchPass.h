// MirEngine/Rendering/Passes/SketchPass.h
// =================================================================================
// Оверлей 2D-эскиза на рабочей плоскости (ТЗ Этап 2).
//
// Для каждого эскиза из RenderContext::sketches рисует отрезки, заданные в
// локальной СК плоскости (origin, xAxis, yAxis). Геометрия пересчитывается на
// CPU и заливается в один динамический буфер (тонкие линии).
// =================================================================================

#pragma once

#include "RenderPass.h"
#include "../Core/RenderContext.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace MirEngine::Rendering
{

class OpenGLShader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class RenderDevice;

class SketchPass final : public RenderPass
{
public:
    SketchPass();
    ~SketchPass() override;

    bool initialize(RenderDevice& device);
    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

    [[nodiscard]] bool isInitialized() const noexcept { return m_initialized; }

private:
    bool m_initialized{false};

    std::unique_ptr<OpenGLShader> m_shader;
    std::shared_ptr<VertexBuffer> m_vbo;
    std::shared_ptr<IndexBuffer> m_ibo;
    std::shared_ptr<VertexArray> m_vao;

    bool createShaders();
    void buildDynamicGeometry(RenderDevice& device,
                              const std::vector<SketchRenderData>& sketches);
};

} // namespace MirEngine::Rendering
