// MirEngine/Rendering/Passes/OverlayPass.cpp
// =================================================================================
// Реализация OverlayPass с заглушками меток осей.
// =================================================================================

#include "OverlayPass.h"
#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Rendering/Core/RenderContext.h"
#include "../Scene/Scene.h"
#include "../Scene/Camera.h"

#include <spdlog/spdlog.h>

namespace MirEngine {
namespace Rendering {

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

OverlayPass::OverlayPass(ShaderLibrary& shaderLibrary)
    : m_shaderLibrary(shaderLibrary)
{
}

bool OverlayPass::initialize(RenderDevice& device) {
    ShaderHandle handle = m_shaderLibrary.load("SolidColor",
                                               kSolidColorVert,
                                               kSolidColorFrag);
    if (handle.empty()) {
        spdlog::error("[OverlayPass] Failed to load shader.");
        return false;
    }
    m_materialHandle = 5000;
    device.registerMaterial(m_materialHandle, m_shaderLibrary.get(handle));

    buildOverlayGeometry();
    m_vb = device.createVertexBuffer();
    m_vb->uploadVertices(m_vertices, BufferUsage::Static);
    m_vao = device.createVertexArray();
    m_vao->setVertexBuffer(m_vb);
    m_meshHandle = 5000;
    device.registerMesh(m_meshHandle, m_vao);

    spdlog::info("[OverlayPass] Initialized.");
    return true;
}

void OverlayPass::execute(RenderContext& /*context*/,
                          Scene& /*scene*/,
                          Camera& /*camera*/,
                          RenderDevice& device) {
    auto shader = m_shaderLibrary.get("SolidColor");
    if (shader) shader->bind();

    RenderCommand cmd;
    cmd.mesh = m_meshHandle;
    cmd.material = m_materialHandle;
    cmd.modelMatrix = IdentityMatrix4(); // метки уже расположены в мировых координатах
    cmd.primitive = PrimitiveType::Lines;
    device.draw(cmd);
}

void OverlayPass::addLine(std::vector<Vertex>& verts,
                          float x1, float y1, float z1,
                          float x2, float y2, float z2,
                          const Vector3& color) {
    verts.push_back({{x1, y1, z1}, color, {}});
    verts.push_back({{x2, y2, z2}, color, {}});
}

void OverlayPass::buildOverlayGeometry() {
    m_vertices.clear();
    float len = 2.0f;        // длина оси (должна совпадать с GizmoPass)
    float offset = 0.3f;     // отступ от конца стрелки
    float size = 0.15f;      // размер буквы
    Vector3 red  = {1,0,0};
    Vector3 green = {0,1,0};
    Vector3 blue  = {0,0,1};

    // Буква X на конце оси X
    float x = len + offset;
    addLine(m_vertices, x-size, -size, 0, x+size, size, 0, red);
    addLine(m_vertices, x-size,  size, 0, x+size, -size, 0, red);

    // Буква Y на конце оси Y
    float y = len + offset;
    addLine(m_vertices, 0, y-size, -size, 0, y, 0, green); // ножка Y
    addLine(m_vertices, 0, y, 0, size, y-size, 0, green);
    addLine(m_vertices, 0, y, 0, -size, y-size, 0, green);

    // Буква Z на конце оси Z
    float z = len + offset;
    addLine(m_vertices, -size, 0, z, size, 0, z, blue);    // верхняя горизонталь
    addLine(m_vertices, -size, 0, z, size, 0, z-size, blue); // диагональ
    addLine(m_vertices, size, 0, z-size, -size, 0, z-size, blue); // нижняя горизонталь
}

} // namespace Rendering
} // namespace MirEngine