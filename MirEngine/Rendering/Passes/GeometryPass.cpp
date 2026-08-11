// MirEngine/Rendering/Passes/GeometryPass.cpp
// =================================================================================
// Реализация GeometryPass с обходом сцены.
// =================================================================================

#include "GeometryPass.h"
#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Resources/Shader.h"
#include "../Rendering/Core/RenderContext.h"
#include "../Scene/Scene.h"
#include "../Scene/Node.h"
#include "../Geometry/Mesh.h"

#include <spdlog/spdlog.h>

namespace MirEngine {
namespace Rendering {

// Простой шейдер с учётом освещения (базовый серый)
static const char* kDefaultVert = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_normal;
out vec3 v_worldPos;

void main() {
    vec4 worldPos = u_model * vec4(aPos, 1.0);
    v_worldPos = worldPos.xyz;
    v_normal = mat3(transpose(inverse(u_model))) * aNormal;
    gl_Position = u_projection * u_view * worldPos;
}
)";

static const char* kDefaultFrag = R"(
#version 410 core
in vec3 v_normal;
in vec3 v_worldPos;
out vec4 FragColor;

uniform vec3 u_lightDir = vec3(0.5, 1.0, 0.3);
uniform vec3 u_color = vec3(0.7, 0.7, 0.7);

void main() {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 ambient = vec3(0.2);
    FragColor = vec4(u_color * (ambient + diff), 1.0);
}
)";

GeometryPass::GeometryPass(ShaderLibrary& shaderLibrary)
    : m_shaderLibrary(shaderLibrary)
{
}

bool GeometryPass::initialize(RenderDevice& device) {
    ShaderHandle handle = m_shaderLibrary.load("DefaultLit",
                                               kDefaultVert,
                                               kDefaultFrag);
    if (handle.empty()) {
        spdlog::error("[GeometryPass] Failed to load default shader.");
        return false;
    }
    m_defaultMaterial = 100; // фикс. дескриптор для стандартного материала
    device.registerMaterial(m_defaultMaterial, m_shaderLibrary.get(handle));
    spdlog::info("[GeometryPass] Initialized with scene traversal.");
    return true;
}

void GeometryPass::execute(RenderContext& context,
                           Scene& scene,
                           Camera& camera,
                           RenderDevice& device) {
    auto shader = m_shaderLibrary.get("DefaultLit");
    if (!shader) {
        spdlog::error("[GeometryPass] DefaultLit shader not found.");
        return;
    }

    // Устанавливаем общие матрицы и направление света
    shader->bind();
    shader->setMatrix("u_view", context.viewMatrix);
    shader->setMatrix("u_projection", context.projectionMatrix);
    shader->setVec3("u_lightDir", 0.5f, 1.0f, 0.3f);

    // Обходим сцену, начиная с корня
    Matrix4Raw identity = IdentityMatrix4();
    processNode(scene.getRoot(), device, identity, shader.get());
}

void GeometryPass::processNode(Node* node, RenderDevice& device,
                               const Matrix4Raw& parentWorld, Shader* shader) {
    if (!node) return;

    Matrix4Raw world = node->getWorldMatrix(); // полная мировая матрица (с учётом иерархии)

    Mesh* mesh = node->getMesh();
    if (mesh) {
        // Проверяем, есть ли уже GPU-ресурсы для этого меша
        auto it = m_meshToHandle.find(mesh);
        MeshHandle handle;
        if (it == m_meshToHandle.end()) {
            // Создаём ресурсы
            auto vb = device.createVertexBuffer();
            vb->uploadVertices(mesh->getVertices(), BufferUsage::Static);
            auto ib = device.createIndexBuffer();
            if (mesh->indexCount() > 0) {
                ib->uploadIndices(mesh->getIndices(), BufferUsage::Static);
            }
            auto vao = device.createVertexArray();
            vao->setVertexBuffer(vb);
            if (mesh->indexCount() > 0) {
                vao->setIndexBuffer(ib);
            }

            handle = m_nextMeshHandle++;
            device.registerMesh(handle, vao);
            m_meshToHandle[mesh] = handle;
            m_vaos[handle] = vao; // сохраняем владение, чтобы ресурсы не удалились
        } else {
            handle = it->second;
        }

        RenderCommand cmd;
        cmd.mesh = handle;
        cmd.material = m_defaultMaterial;
        cmd.modelMatrix = world;
        cmd.primitive = PrimitiveType::Triangles;
        device.draw(cmd);
    }

    // Рекурсивно обходим детей
    for (auto& child : node->getChildren()) {
        processNode(child.get(), device, world, shader);
    }
}

} // namespace Rendering
} // namespace MirEngine