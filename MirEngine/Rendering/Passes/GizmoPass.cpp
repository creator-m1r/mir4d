// MirEngine/Rendering/Passes/GizmoPass.cpp
// =================================================================================
// Реализация GizmoPass.
// =================================================================================

#include "GizmoPass.h"
#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"
#include "../Resources/Vertex.h"

#include <iostream>

namespace MirEngine {
namespace Rendering {

// Тот же шейдер SolidColor, что и в GridPass (можно переиспользовать)
static const char* kSolidColorVert = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
void main() {
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
)";

static const char* kSolidColorFrag = R"(
#version 410 core
out vec4 FragColor;
uniform vec4 u_color;
void main() {
    FragColor = u_color;
}
)";

GizmoPass::GizmoPass(ShaderLibrary& shaderLibrary)
    : m_shaderLibrary(shaderLibrary)
{
}

bool GizmoPass::initialize(RenderDevice& device) {
    // Загружаем шейдер (можно тот же, что и у сетки)
    ShaderHandle shaderHandle = m_shaderLibrary.load("SolidColor",
                                                     kSolidColorVert,
                                                     kSolidColorFrag);
    if (shaderHandle.empty()) {
        std::cerr << "[GizmoPass] Failed to load shader.\n";
        return false;
    }
    m_materialHandle = 4000; // дескриптор для гизмо
    device.registerMaterial(m_materialHandle, m_shaderLibrary.get(shaderHandle));

    // Создаём геометрию
    buildGizmoGeometry();
    m_vb = device.createVertexBuffer();
    m_vb->uploadVertices(m_vertices, BufferUsage::Static);
    m_vao = device.createVertexArray();
    m_vao->setVertexBuffer(m_vb);
    m_meshHandle = 4000;
    device.registerMesh(m_meshHandle, m_vao);

    std::cout << "[GizmoPass] Initialized.\n";
    return true;
}

void GizmoPass::execute(RenderContext& /*context*/,
                        mir::Scene& /*scene*/,
                        RenderDevice& device) {
    auto shader = m_shaderLibrary.get("SolidColor");
    if (shader) shader->bind();

    // Рисуем оси гизмо цветными линиями (без теста глубины, чтобы всегда поверх)
    // Устройство должно будет отключить GL_DEPTH_TEST для этого прохода.
    // Можно передать флаг в RenderCommand или временно установить состояние.
    // Пока просто рисуем с включённой глубиной (будет видно, как они пересекаются).

    RenderCommand cmd;
    cmd.mesh = m_meshHandle;
    cmd.material = m_materialHandle;
    cmd.modelMatrix = IdentityMatrix4(); // в будущем будет пересчитываться к позиции объекта
    // Смещаем к позиции гизмо через modelMatrix (пока позиция (0,0,0))
    cmd.primitive = PrimitiveType::Lines;
    device.draw(cmd);
}

void GizmoPass::buildGizmoGeometry() {
    m_vertices.clear();
    float len = 2.0f; // длина оси
    float arrowSize = 0.2f;

    // Ось X (красный)
    m_vertices.push_back({{0,0,0}, {1,0,0}, {}});
    m_vertices.push_back({{len,0,0}, {1,0,0}, {}});
    // стрелка X
    m_vertices.push_back({{len,0,0}, {1,0,0}, {}});
    m_vertices.push_back({{len-arrowSize, arrowSize, 0}, {1,0,0}, {}});
    m_vertices.push_back({{len,0,0}, {1,0,0}, {}});
    m_vertices.push_back({{len-arrowSize, -arrowSize, 0}, {1,0,0}, {}});

    // Ось Y (зелёный)
    m_vertices.push_back({{0,0,0}, {0,1,0}, {}});
    m_vertices.push_back({{0,len,0}, {0,1,0}, {}});
    m_vertices.push_back({{0,len,0}, {0,1,0}, {}});
    m_vertices.push_back({{arrowSize, len-arrowSize, 0}, {0,1,0}, {}});
    m_vertices.push_back({{0,len,0}, {0,1,0}, {}});
    m_vertices.push_back({{-arrowSize, len-arrowSize, 0}, {0,1,0}, {}});

    // Ось Z (синий)
    m_vertices.push_back({{0,0,0}, {0,0,1}, {}});
    m_vertices.push_back({{0,0,len}, {0,0,1}, {}});
    m_vertices.push_back({{0,0,len}, {0,0,1}, {}});
    m_vertices.push_back({{arrowSize, 0, len-arrowSize}, {0,0,1}, {}});
    m_vertices.push_back({{0,0,len}, {0,0,1}, {}});
    m_vertices.push_back({{-arrowSize, 0, len-arrowSize}, {0,0,1}, {}});
}

} // namespace Rendering
} // namespace MirEngine