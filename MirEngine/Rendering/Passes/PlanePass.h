// MirEngine/Rendering/Passes/PlanePass.h
// =================================================================================
// Оверлей рабочих плоскостей (ТЗ Этап 1, раздел 5).
//
// Для каждой плоскости из RenderContext::planes рисует:
//   - полупрозрачную рабочую поверхность (квадрат в локальной СК плоскости);
//   - локальную ось X (красная), Y (зелёная) и нормаль (синяя);
//   - подсветку активной/выбранной плоскости.
//
// Геометрия пересчитывается на CPU из basis плоскости и заливается в один
// динамический буфер (несколько десятков вершин — пренебрежимо).
// =================================================================================

#pragma once

#include "RenderPass.h"
#include "../Core/RenderContext.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace MirEngine::Rendering
{

class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class RenderDevice;
class OpenGLShader;

class PlanePass final : public RenderPass
{
public:
    PlanePass();
    ~PlanePass() override;

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
                              const std::vector<PlaneRenderData>& planes);
};

} // namespace MirEngine::Rendering
