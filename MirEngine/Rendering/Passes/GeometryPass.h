// MirEngine/Rendering/Passes/GeometryPass.h
// =================================================================================
// Проход рендеринга основной геометрии (Geometry Pass).
//
// Отвечает за отрисовку трёхмерных объектов сцены с применением материалов и
// освещения. На данном этапе реализует простейший тестовый треугольник, чтобы
// замкнуть вертикаль рендеринга и проверить работу всей цепочки:
//   Renderer → Pass → RenderCommand → RenderDevice → OpenGL.
//
// В будущем будет заменён на полноценный обход графа сцены, сбор команд и
// сортировку по материалам/прозрачности.
//
// Архитектура:
//   - Не зависит от OpenGL; использует абстракции RenderDevice, Shader, VertexArray.
//   - Получает ShaderLibrary через конструктор для загрузки шейдеров.
//   - Создаёт тестовую геометрию в методе initialize() и регистрирует меш в устройстве.
//   - В execute() формирует RenderCommand с модельной матрицей и вызывает device->draw().
// =================================================================================
// Проход рендеринга основной геометрии (Geometry Pass) — версия с обходом сцены.
// =================================================================================

#pragma once

#include "RenderPass.h"
#include "../Core/RenderCommand.h"
#include "../Resources/ShaderLibrary.h"
#include <memory>
#include <unordered_map>

namespace MirEngine {

class Mesh;
class Node;
class Scene;
class Camera;

namespace Rendering {

class RenderDevice;
class RenderContext;
class VertexArray;

class GeometryPass : public RenderPass {
public:
    explicit GeometryPass(ShaderLibrary& shaderLibrary);

    bool initialize(RenderDevice& device);
    void execute(RenderContext& context,
                 Scene& scene,
                 Camera& camera,
                 RenderDevice& device) override;

private:
    ShaderLibrary& m_shaderLibrary;
    MaterialHandle m_defaultMaterial = 0;
    MeshHandle m_nextMeshHandle = 1; // простой автоинкремент для дескрипторов

    // Кэш: Mesh* -> MeshHandle (чтобы не дублировать GPU-ресурсы)
    std::unordered_map<Mesh*, MeshHandle> m_meshToHandle;
    // Кэш: MeshHandle -> VAO (владение ресурсами)
    std::unordered_map<MeshHandle, std::shared_ptr<VertexArray>> m_vaos;

    // Рекурсивный обход узлов
    void processNode(Node* node, RenderDevice& device, const Matrix4Raw& parentWorld,
                     Shader* shader);
};

} // namespace Rendering
} // namespace MirEngine