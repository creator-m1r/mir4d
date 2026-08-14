// MirEngine/Rendering/Passes/GizmoPass.h
// =================================================================================
// Проход отрисовки гизмо (Gizmo Pass).
//
// Рисует 3D-манипуляторы (стрелки перемещения, дуги вращения, кубы масштаба)
// поверх сцены. На данный момент реализует простые оси со стрелками (X — красный,
// Y — зелёный, Z — синий) и центральный куб. В будущем будет взаимодействовать
// с системой трансформаций и Selection для перемещения/вращения объектов.
//
// Архитектура:
//   - Не зависит от платформы и рендеринг-бэкенда.
//   - Создаёт геометрию стрелок при инициализации.
//   - Использует шейдер "SolidColor" из ShaderLibrary.
//
// Особенности:
//   - Гизмо должно рисоваться с отключённым тестом глубины (всегда поверх сцены).
//   - Размер гизмо может быть фиксированным в экранных координатах или мировых.
// =================================================================================

#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/ShaderLibrary.h"
#include <memory>
#include <vector>

namespace MirEngine {
namespace Rendering {

class RenderDevice;
class RenderContext;
class VertexArray;
class VertexBuffer;

class GizmoPass : public RenderPass {
public:
    explicit GizmoPass(ShaderLibrary& shaderLibrary);

    bool initialize(RenderDevice& device);

    void execute(RenderContext& context,
                 Scene& scene,
                 Camera& camera,
                 RenderDevice& device) override;

    // Позиция гизмо (в будущем будет задаваться выделенным объектом)
    void setPosition(const Vector3& position) { m_position = position; }

private:
    ShaderLibrary& m_shaderLibrary;

    MaterialHandle m_materialHandle = 0;
    MeshHandle     m_meshHandle = 0;

    std::shared_ptr<VertexArray>  m_vao;
    std::shared_ptr<VertexBuffer> m_vb;

    Vector3 m_position = {0.0f, 0.0f, 0.0f};

    // Построение геометрии осей
    void buildGizmoGeometry();
};

} // namespace Rendering
} // namespace MirEngine