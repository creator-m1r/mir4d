// MirEngine/Rendering/Passes/SelectionPass.cpp
// =================================================================================
// Реализация SelectionPass (тестовый контур).
// =================================================================================

#include "SelectionPass.h"
#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Rendering/Core/RenderContext.h"
#include "../Scene/Scene.h"
#include "../Scene/Camera.h"

#include <spdlog/spdlog.h>

namespace MirEngine {
namespace Rendering {

// Шейдер для выделения (одноцветный, яркий)
static const char* kSelectionVert = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
void main() {
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}
)";

static const char* kSelectionFrag = R"(
#version 410 core
out vec4 FragColor;
uniform vec4 u_color;
void main() {
    FragColor = u_color;
}
)";

SelectionPass::SelectionPass(ShaderLibrary& shaderLibrary)
    : m_shaderLibrary(shaderLibrary)
{
}

bool SelectionPass::initialize(RenderDevice& device) {
    // Загрузка шейдера
    ShaderHandle shaderHandle = m_shaderLibrary.load("SelectionColor",
                                                     kSelectionVert,
                                                     kSelectionFrag);
    if (shaderHandle.empty()) {
        spdlog::error("[SelectionPass] Failed to load shader.");
        return false;
    }
    m_materialHandle = 3000; // фиксированный дескриптор
    device.registerMaterial(m_materialHandle, m_shaderLibrary.get(shaderHandle));

    // Создаём тестовый контур: квадрат на плоскости XY (4 линии)
    // В будущем будет заменён на обход выделенных объектов.
    m_vertices.clear();
    float s = 0.6f; // размер квадрата
    m_vertices.push_back({{-s, -s, 0.01f}, {0,0,1}, {}});
    m_vertices.push_back({{ s, -s, 0.01f}, {0,0,1}, {}});
    m_vertices.push_back({{ s,  s, 0.01f}, {0,0,1}, {}});
    m_vertices.push_back({{-s,  s, 0.01f}, {0,0,1}, {}});

    // Буфер и VAO
    m_vb = device.createVertexBuffer();
    m_vb->uploadVertices(m_vertices, BufferUsage::Static);
    m_vao = device.createVertexArray();
    m_vao->setVertexBuffer(m_vb);
    m_meshHandle = 3000;
    device.registerMesh(m_meshHandle, m_vao);

    spdlog::info("[SelectionPass] Initialized with test contour.");
    return true;
}

void SelectionPass::execute(RenderContext& context,
                            Scene& /*scene*/,
                            Camera& /*camera*/,
                            RenderDevice& device) {
    // Устанавливаем ярко-жёлтый цвет
    auto shader = m_shaderLibrary.get("SelectionColor");
    if (shader) {
        shader->bind();
        shader->setVec4("u_color", 1.0f, 0.8f, 0.0f, 1.0f);
    }

    RenderCommand cmd;
    cmd.mesh = m_meshHandle;
    cmd.material = m_materialHandle;
    cmd.modelMatrix = IdentityMatrix4();
    cmd.primitive = PrimitiveType::LineLoop; // рисуем замкнутый контур
    cmd.wireframe = false; // LineLoop и так линии

    device.draw(cmd);
}

} // namespace Rendering
} // namespace MirEngine