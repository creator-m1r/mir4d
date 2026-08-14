// MirEngine/Rendering/Passes/OverlayPass.h
// =================================================================================
// Проход оверлеев (Overlay Pass).
//
// Рисует вспомогательную информацию поверх 3D-сцены:
//   - Буквы X, Y, Z на концах осей гизмо/сетки.
//   - Размерные линии, подписи, центры, точки привязки (в будущем).
//
// На данный момент реализует заглушки меток осей — простые линии, образующие
// буквы. Метки всегда смотрят на камеру (billboarding) для читаемости;
// биллбординг реализуется через шейдер (пока не включён, но будет доработан).
//
// Архитектура:
//   - Использует тот же шейдер SolidColor.
//   - Геометрия меток (несколько линий на букву) загружается при инициализации.
//   - В execute() устанавливает позиции меток в соответствии с длинами осей.
// =================================================================================

#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/ShaderLibrary.h"
#include "../Resources/Vertex.h"
#include <memory>
#include <vector>

namespace MirEngine {

class Camera;

namespace Rendering {

class RenderDevice;
class RenderContext;
class VertexArray;
class VertexBuffer;

class OverlayPass : public RenderPass {
public:
    explicit OverlayPass(ShaderLibrary& shaderLibrary);

    bool initialize(RenderDevice& device);

    void execute(RenderContext& context,
                 mir::Scene& scene,
                 RenderDevice& device) override;

private:
    ShaderLibrary& m_shaderLibrary;

    MaterialHandle m_materialHandle = 0;
    MeshHandle     m_meshHandle = 0;

    std::shared_ptr<VertexArray>  m_vao;
    std::shared_ptr<VertexBuffer> m_vb;

    std::vector<Vertex> m_vertices;

    // Вспомогательный метод для добавления сегмента линии в список вершин
    void addLine(std::vector<Vertex>& verts,
                 float x1, float y1, float z1,
                 float x2, float y2, float z2,
                 const Vector3& color);
    void buildOverlayGeometry();
};

} // namespace Rendering
} // namespace MirEngine