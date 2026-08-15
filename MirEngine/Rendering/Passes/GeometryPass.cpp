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

#include <cmath>
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

// All lighting vectors and normals are evaluated in VIEW space. The model
// translation is camera-relative and therefore stays numerically stable for
// large CAD coordinates.
static const char* kDefaultVert = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_normal;
out vec3 v_viewPos;

void main()
{
    vec4 modelPos = u_model * vec4(aPos, 1.0);
    vec4 viewPos = u_view * modelPos;

    // u_view is rotation-only. The inverse-transpose keeps normals correct
    // for future non-uniform CAD object scales as well as rotations.
    mat3 normalMatrix = mat3(transpose(inverse(u_view * u_model)));
    v_normal = normalize(normalMatrix * aNormal);
    v_viewPos = viewPos.xyz;

    gl_Position = u_projection * viewPos;
}
)";

// Engineering CAD lighting: GGX specular + Lambert diffuse + Schlick
// Fresnel, with a warm key, cool fill and soft ambient contribution.
static const char* kDefaultFrag = R"(
#version 410 core
in vec3 v_normal;
in vec3 v_viewPos;
out vec4 FragColor;

// Directions point FROM the surface TOWARD each light and are already in
// view space. Keeping every lighting vector in one coordinate space avoids
// the previous world/view mismatch that caused unstable or dark faces.
uniform vec3 u_keyDir;
uniform vec3 u_keyColor;
uniform float u_keyIntensity;
uniform vec3 u_fillDir;
uniform vec3 u_fillColor;
uniform float u_fillIntensity;
uniform vec3 u_ambientColor;
uniform float u_ambientIntensity;

uniform vec3 u_baseColor;
uniform float u_roughness;
uniform float u_metallic;
uniform float u_specular;
uniform float u_ior;
uniform vec3 u_emission;
uniform float u_opacity;

const float PI = 3.14159265359;

float D_GGX(float NoH, float alpha)
{
    float a2 = alpha * alpha;
    float d = NoH * NoH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * d * d, 1e-6);
}

float V_Smith(float NoV, float NoL, float alpha)
{
    float a2 = alpha * alpha;
    float gv = NoL * sqrt(max(NoV * NoV * (1.0 - a2) + a2, 1e-6));
    float gl = NoV * sqrt(max(NoL * NoL * (1.0 - a2) + a2, 1e-6));
    return 0.5 / max(gv + gl, 1e-5);
}

vec3 F_Schlick(float VoH, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}

vec3 shadeDirectional(vec3 N, vec3 V, vec3 L,
                      vec3 lightColor, float lightIntensity,
                      vec3 baseColor, float metallic,
                      float roughness, vec3 F0)
{
    L = normalize(L);
    float NoL = clamp(dot(N, L), 0.0, 1.0);
    if (NoL <= 0.0)
        return vec3(0.0);

    vec3 H = normalize(L + V);
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);

    float alpha = max(roughness * roughness, 0.02);
    float D = D_GGX(NoH, alpha);
    float Vis = V_Smith(NoV, NoL, alpha);
    vec3 F = F_Schlick(VoH, F0);

    vec3 specular = D * Vis * F;
    vec3 diffuseWeight = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = diffuseWeight * baseColor / PI;

    return (diffuse + specular) * lightColor * lightIntensity * NoL;
}

void main()
{
    vec3 N = normalize(v_normal);
    vec3 V = normalize(-v_viewPos);

    // Metallic materials use their base color as F0; dielectric materials
    // start from the artist-controlled specular value.
    float dielectricF0 = clamp(0.04 * max(u_specular, 0.0), 0.0, 0.16);
    vec3 F0 = mix(vec3(dielectricF0), clamp(u_baseColor, 0.0, 1.0),
                  clamp(u_metallic, 0.0, 1.0));

    vec3 Lo = vec3(0.0);
    Lo += shadeDirectional(N, V, u_keyDir, u_keyColor, u_keyIntensity,
                           u_baseColor, u_metallic, u_roughness, F0);
    Lo += shadeDirectional(N, V, u_fillDir, u_fillColor, u_fillIntensity,
                           u_baseColor, u_metallic, u_roughness, F0);

    vec3 ambient = u_ambientColor * u_ambientIntensity * u_baseColor;
    vec3 color = max(ambient + Lo + u_emission, vec3(0.0));

    // Filmic-friendly simple tonemap followed by display gamma.
    color = color / (color + vec3(1.0));
    color = pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));

    FragColor = vec4(color, clamp(u_opacity, 0.0, 1.0));
}
)";

namespace
{

struct EngineeringStudioLights
{
    float keyDir[3] = {0.50f, 1.00f, 0.30f};
    float keyColor[3] = {1.00f, 0.98f, 0.94f};
    float keyIntensity = 1.10f;

    float fillDir[3] = {-0.60f, -0.20f, -0.80f};
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

    Matrix4Raw viewRel = context.viewMatrix;
    // Matrix4Raw is column-major: translation is [12,13,14].
    viewRel[12] = 0.0f;
    viewRel[13] = 0.0f;
    viewRel[14] = 0.0f;

    EngineeringStudioLights lights;
    normalize3(lights.keyDir);
    normalize3(lights.fillDir);

    // Transform world-space light directions into view space using only the
    // camera rotation. Normals and view vectors use that same space below.
    const float keyDirView[3] = {
        viewRel[0] * lights.keyDir[0] + viewRel[4] * lights.keyDir[1] + viewRel[8] * lights.keyDir[2],
        viewRel[1] * lights.keyDir[0] + viewRel[5] * lights.keyDir[1] + viewRel[9] * lights.keyDir[2],
        viewRel[2] * lights.keyDir[0] + viewRel[6] * lights.keyDir[1] + viewRel[10] * lights.keyDir[2]};
    const float fillDirView[3] = {
        viewRel[0] * lights.fillDir[0] + viewRel[4] * lights.fillDir[1] + viewRel[8] * lights.fillDir[2],
        viewRel[1] * lights.fillDir[0] + viewRel[5] * lights.fillDir[1] + viewRel[9] * lights.fillDir[2],
        viewRel[2] * lights.fillDir[0] + viewRel[6] * lights.fillDir[1] + viewRel[10] * lights.fillDir[2]};

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

    for (const auto& node : scene.nodes())
    {
        if (!node || !node->model() || !node->model()->hasMesh())
            continue;
        if (node->id() == mir4d::InvalidObjectId)
            continue;

        processNode(*node, device, shader.get(), context.cameraPosition);
    }
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
    const mir::Matrix4 matrix = transform.matrix();
    Matrix4Raw result{};
    for (std::size_t row = 0; row < 4; ++row)
    {
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
    }
    return result;
}

} // namespace MirEngine::Rendering
