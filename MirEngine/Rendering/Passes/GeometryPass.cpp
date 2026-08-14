#include "GeometryPass.h"

#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Resources/Shader.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"

#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Geometry/Model/ModelNode.hpp"
#include "MirEngine/Geometry/Tessellation/TriangleMesh.hpp"
#include "MirEngine/Math/Transform.hpp"

#include <spdlog/spdlog.h>

#include <cstdint>
#include <vector>

namespace MirEngine::Rendering
{

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

bool GeometryPass::initialize(RenderDevice& device)
{
    const ShaderHandle handle = m_shaderLibrary.load("DefaultLit", kDefaultVert, kDefaultFrag);
    if (handle.empty())
    {
        spdlog::error("[GeometryPass] Failed to load default shader.");
        return false;
    }

    m_defaultMaterial = 100;
    device.registerMaterial(m_defaultMaterial, m_shaderLibrary.get(handle));
    return true;
}

void GeometryPass::invalidateSceneCache() noexcept
{
    m_objectToHandle.clear();
    m_vaos.clear();
    m_nextMeshHandle = 1;
    m_cachedScene = nullptr;
    m_cachedRevision = 0;
}

void GeometryPass::execute(RenderContext& context,
                           mir::Scene& scene,
                           RenderDevice& device)
{
    auto shader = m_shaderLibrary.get("DefaultLit");
    if (!shader)
    {
        spdlog::error("[GeometryPass] DefaultLit shader not found.");
        return;
    }

    const std::uint64_t revision = scene.contentRevision();
    if (m_cachedScene != &scene || m_cachedRevision != revision)
    {
        m_objectToHandle.clear();
        m_vaos.clear();
        m_nextMeshHandle = 1;
        m_cachedScene = &scene;
        m_cachedRevision = revision;
    }

    shader->bind();
    shader->setMatrix("u_view", context.viewMatrix);
    shader->setMatrix("u_projection", context.projectionMatrix);
    shader->setVec3("u_lightDir", 0.5f, 1.0f, 0.3f);

    for (const auto& node : scene.nodes())
    {
        if (!node || !node->model() || !node->model()->hasMesh())
            continue;
        if (node->id() == mir::InvalidObjectId)
            continue;

        processNode(*node, device, shader.get());
    }
}

void GeometryPass::processNode(const mir::ModelNode& node,
                               RenderDevice& device,
                               Shader* shader)
{
    const mir::TriangleMesh3& mesh = node.model()->mesh();
    if (!mesh.isValid())
        return;

    MeshHandle handle = 0;
    const auto cached = m_objectToHandle.find(node.id());
    if (cached != m_objectToHandle.end())
    {
        handle = cached->second;
    }
    else
    {
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        vertices.reserve(mesh.vertices.size());
        indices.reserve(mesh.triangles.size() * 3);

        std::vector<mir::Vector3> normals;
        if (mesh.hasVertexNormals())
        {
            normals = mesh.normals;
        }
        else
        {
            normals.assign(mesh.vertices.size(), mir::Vector3::zero());
            for (const auto& triangle : mesh.triangles)
            {
                const mir::Vector3 a{
                    mesh.vertices[triangle.a].x,
                    mesh.vertices[triangle.a].y,
                    mesh.vertices[triangle.a].z};
                const mir::Vector3 b{
                    mesh.vertices[triangle.b].x,
                    mesh.vertices[triangle.b].y,
                    mesh.vertices[triangle.b].z};
                const mir::Vector3 c{
                    mesh.vertices[triangle.c].x,
                    mesh.vertices[triangle.c].y,
                    mesh.vertices[triangle.c].z};
                const mir::Vector3 n = mir::Vector3::cross(b - a, c - a).normalized();
                normals[triangle.a] += n;
                normals[triangle.b] += n;
                normals[triangle.c] += n;
            }
            for (auto& normal : normals)
                normal = normal.normalized();
        }

        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            const auto& p = mesh.vertices[i];
            const auto& n = normals[i];
            vertices.emplace_back(
                Rendering::Vector3{static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(p.z)},
                Rendering::Vector3{static_cast<float>(n.x), static_cast<float>(n.y), static_cast<float>(n.z)},
                Rendering::Vector2{});
        }

        for (const auto& triangle : mesh.triangles)
        {
            indices.push_back(static_cast<std::uint32_t>(triangle.a));
            indices.push_back(static_cast<std::uint32_t>(triangle.b));
            indices.push_back(static_cast<std::uint32_t>(triangle.c));
        }

        auto vb = device.createVertexBuffer();
        vb->uploadVertices(vertices, BufferUsage::Static);
        auto ib = device.createIndexBuffer();
        ib->uploadIndices(indices, BufferUsage::Static);
        auto vao = device.createVertexArray();
        vao->setVertexBuffer(vb);
        vao->setIndexBuffer(ib);

        handle = m_nextMeshHandle++;
        device.registerMesh(handle, vao);
        m_objectToHandle[node.id()] = handle;
        m_vaos[handle] = vao;
    }

    RenderCommand command;
    command.mesh = handle;
    command.material = m_defaultMaterial;
    command.modelMatrix = makeModelMatrix(node.transform());
    command.primitive = PrimitiveType::Triangles;
    device.draw(command);

    (void)shader;
}

Matrix4Raw GeometryPass::makeModelMatrix(const mir::Transform& transform) noexcept
{
    const mir::Matrix4 matrix = transform.matrix();
    Matrix4Raw result{};
    for (std::size_t row = 0; row < 4; ++row)
        for (std::size_t column = 0; column < 4; ++column)
            result[row + column * 4] = static_cast<float>(matrix(row, column));
    return result;
}

} // namespace MirEngine::Rendering
