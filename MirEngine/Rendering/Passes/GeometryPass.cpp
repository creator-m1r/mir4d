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

#include <iostream>
#include <cstdint>
#include <vector>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine::Rendering
{

// Camera-relative vertex shader:
//   u_model carries world translation already shifted by the camera position
//   (computed in double on the CPU), u_view keeps only the rotation part,
//   so all GPU numbers stay small regardless of world scale.
static const char* kDefaultVert = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_normal;
out vec3 v_worldPos;
out vec3 v_viewPos;

void main() {
    vec4 worldPos = u_model * vec4(aPos, 1.0);
    v_worldPos = worldPos.xyz;
    v_normal = mat3(u_model) * aNormal;
    vec4 viewPos = u_view * worldPos;
    v_viewPos = viewPos.xyz;
    gl_Position = u_projection * viewPos;
}
)";

// PBR fragment shader:
//   GGX specular + Lambert diffuse + Schlick Fresnel,
//   Engineering Studio light rig (key + fill + ambient),
//   fully procedural material parameters (no textures).
static const char* kDefaultFrag = R"(
#version 410 core
in vec3 v_normal;
in vec3 v_worldPos;
in vec3 v_viewPos;
out vec4 FragColor;

uniform vec3 u_keyDir;      // world-space key light direction
uniform vec3 u_keyColor;
uniform float u_keyIntensity;
uniform vec3 u_fillDir;
uniform vec3 u_fillColor;
uniform float u_fillIntensity;
uniform vec3 u_ambientColor;
uniform float u_ambientIntensity;
uniform mat3 u_worldToView;

uniform vec3 u_baseColor;
uniform float u_roughness;
uniform float u_metallic;
uniform float u_specular;
uniform float u_ior;
uniform vec3 u_emission;
uniform float u_opacity;

const float PI = 3.14159265359;

float D_GGX(float NoH, float a)
{
    float a2 = a * a;
    float d = NoH * NoH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float V_Smith(float NoV, float NoL, float a)
{
    float a2 = a * a;
    float gv = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float gl = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (gv + gl + 1e-5);
}

vec3 F_Schlick(float u, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - u, 5.0);
}

vec3 shadeDirectional(vec3 N, vec3 V, vec3 Ldir, vec3 lightColor, float lightIntensity,
                      vec3 baseColor, float metallic, float roughness, vec3 F0)
{
    vec3 L = normalize(u_worldToView * Ldir);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    if (NoL <= 0.0) return vec3(0.0);

    vec3 H = normalize(L + V);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float NoV = clamp(dot(N, V), 0.0, 1.0);

    float a = max(roughness * roughness, 0.02);
    float D = D_GGX(NoH, a);
    float Vis = V_Smith(NoV, NoL, a);
    vec3 F = F_Schlick(clamp(dot(V, H), 0.0, 1.0), F0);

    vec3 spec = D * Vis * F;
    vec3 kd = (1.0 - F) * (1.0 - metallic);
    return (kd * baseColor / PI + spec) * lightColor * lightIntensity * NoL;
}

void main() {
    vec3 N = normalize(v_normal);
    vec3 V = normalize(-v_viewPos);
    float NoV = clamp(dot(N, V), 0.0, 1.0);

    vec3 F0 = mix(vec3(0.04 * u_specular), u_baseColor, u_metallic);

    vec3 Lo = vec3(0.0);
    Lo += shadeDirectional(N, V, u_keyDir, u_keyColor, u_keyIntensity,
                           u_baseColor, u_metallic, u_roughness, F0);
    Lo += shadeDirectional(N, V, u_fillDir, u_fillColor, u_fillIntensity,
                           u_baseColor, u_metallic, u_roughness, F0);

    vec3 ambient = u_ambientColor * u_ambientIntensity * u_baseColor;

    vec3 color = ambient + Lo + u_emission;
    color = max(color, vec3(0.0));
    color = color / (color + vec3(1.0)); // soft tone mapping
    color = pow(color, vec3(1.0 / 2.2)); // gamma

    FragColor = vec4(color, u_opacity);
}
)";

namespace
{

/// Engineering Studio light rig: soft key + cool fill + ambient floor.
/// Guarantees readable faces and no fully black surfaces.
struct EngineeringStudioLights
{
    float keyDir[3] = {0.5f, 1.0f, 0.3f};
    float keyColor[3] = {1.0f, 0.98f, 0.94f};
    float keyIntensity = 1.1f;

    float fillDir[3] = {-0.6f, -0.2f, -0.8f};
    float fillColor[3] = {0.62f, 0.70f, 0.85f};
    float fillIntensity = 0.45f;

    float ambientColor[3] = {0.42f, 0.45f, 0.50f};
    float ambientIntensity = 0.60f;
};

void normalize3(float v[3]) noexcept
{
    const float len = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len > 1e-9f)
    {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

} // namespace

GeometryPass::GeometryPass(ShaderLibrary& shaderLibrary)
    : m_shaderLibrary(shaderLibrary)
{
}

bool GeometryPass::initialize(RenderDevice& device)
{
    const ShaderHandle handle = m_shaderLibrary.load("DefaultLit", kDefaultVert, kDefaultFrag);
    if (handle.empty())
    {
        std::cerr << "[GeometryPass] Failed to load default shader.\n";
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
        std::cerr << "[GeometryPass] DefaultLit shader not found.\n";
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

    // Camera-relative view matrix: rotation only, no translation.
    Matrix4Raw viewRel = context.viewMatrix;
    viewRel[3] = 0.0f; viewRel[7] = 0.0f; viewRel[11] = 0.0f;

    EngineeringStudioLights lights;
    normalize3(lights.keyDir);
    normalize3(lights.fillDir);

    // World directions -> view space (for consistent half vectors).
    float keyDirView[3] = {
        viewRel[0] * lights.keyDir[0] + viewRel[4] * lights.keyDir[1] + viewRel[8] * lights.keyDir[2],
        viewRel[1] * lights.keyDir[0] + viewRel[5] * lights.keyDir[1] + viewRel[9] * lights.keyDir[2],
        viewRel[2] * lights.keyDir[0] + viewRel[6] * lights.keyDir[1] + viewRel[10] * lights.keyDir[2]};
    float fillDirView[3] = {
        viewRel[0] * lights.fillDir[0] + viewRel[4] * lights.fillDir[1] + viewRel[8] * lights.fillDir[2],
        viewRel[1] * lights.fillDir[0] + viewRel[5] * lights.fillDir[1] + viewRel[9] * lights.fillDir[2],
        viewRel[2] * lights.fillDir[0] + viewRel[6] * lights.fillDir[1] + viewRel[10] * lights.fillDir[2]};

    float worldToView[9] = {
        viewRel[0], viewRel[1], viewRel[2],
        viewRel[4], viewRel[5], viewRel[6],
        viewRel[8], viewRel[9], viewRel[10]};

    shader->bind();
    shader->setMatrix("u_view", viewRel);
    shader->setMatrix("u_projection", context.projectionMatrix);
    shader->setVec3("u_keyDir", keyDirView[0], keyDirView[1], keyDirView[2]);
    shader->setVec3("u_keyColor", lights.keyColor[0], lights.keyColor[1], lights.keyColor[2]);
    shader->setFloat("u_keyIntensity", lights.keyIntensity);
    shader->setVec3("u_fillDir", fillDirView[0], fillDirView[1], fillDirView[2]);
    shader->setVec3("u_fillColor", lights.fillColor[0], lights.fillColor[1], lights.fillColor[2]);
    shader->setFloat("u_fillIntensity", lights.fillIntensity);
    shader->setVec3("u_ambientColor", lights.ambientColor[0], lights.ambientColor[1], lights.ambientColor[2]);
    shader->setFloat("u_ambientIntensity", lights.ambientIntensity);
    shader->setMatrix3("u_worldToView", worldToView);

    const double cameraPosD[3] = {
        static_cast<double>(context.cameraPosition[0]),
        static_cast<double>(context.cameraPosition[1]),
        static_cast<double>(context.cameraPosition[2])};

    for (const auto& node : scene.nodes())
    {
        if (!node || !node->model() || !node->model()->hasMesh())
            continue;
        if (node->id() == mir4d::InvalidObjectId)
            continue;

        processNode(*node, device, shader.get(), context.cameraPosition);
    }

    (void)cameraPosD;
}

void GeometryPass::processNode(const mir::ModelNode& node,
                               RenderDevice& device,
                               Shader* shader,
                               const float cameraPos[3])
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

    // Material resolution: object -> library material (default Engineering Steel).
    MaterialId materialId = MaterialLibrary::defaultMaterial();
    const auto materialIt = m_objectMaterials.find(node.id());
    if (materialIt != m_objectMaterials.end())
        materialId = materialIt->second;
    const MaterialData material = MaterialLibrary::material(materialId);

    shader->setVec3("u_baseColor", material.baseColor[0], material.baseColor[1], material.baseColor[2]);
    shader->setFloat("u_roughness", material.roughness);
    shader->setFloat("u_metallic", material.metallic);
    shader->setFloat("u_specular", material.specular);
    shader->setFloat("u_ior", material.ior);
    shader->setVec3("u_emission", material.emission[0], material.emission[1], material.emission[2]);
    shader->setFloat("u_opacity", material.opacity);

    RenderCommand command;
    command.mesh = handle;
    command.material = m_defaultMaterial;
    const double cameraPosD[3] = {
        static_cast<double>(cameraPos[0]),
        static_cast<double>(cameraPos[1]),
        static_cast<double>(cameraPos[2])};
    command.modelMatrix = makeModelMatrix(node.transform(), cameraPosD);
    command.primitive = PrimitiveType::Triangles;
    device.draw(command);

    (void)shader;
}

Matrix4Raw GeometryPass::makeModelMatrix(const mir::Transform& transform,
                                         const double cameraPos[3]) noexcept
{
    // Camera-relative model matrix: translation shifted by the camera
    // position in double precision, so GPU floats stay small.
    const mir::Matrix4 matrix = transform.matrix();
    Matrix4Raw result{};
    for (std::size_t row = 0; row < 4; ++row)
        for (std::size_t column = 0; column < 4; ++column)
        {
            if (column == 3 && row < 3)
            {
                const double shifted =
                    static_cast<double>(matrix(row, column)) - cameraPos[row];
                result[row + column * 4] = static_cast<float>(shifted);
            }
            else
            {
                result[row + column * 4] = static_cast<float>(matrix(row, column));
            }
        }
    return result;
}

} // namespace MirEngine::Rendering