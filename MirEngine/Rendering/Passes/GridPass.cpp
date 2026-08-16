// MirEngine/Rendering/Passes/GridPass.cpp
// =================================================================================
// Процедурная инженерная сетка рабочей плоскости.
//
// Сетка рисуется как один full-screen треугольник; фрагментный шейдер
// восстанавливает мировую точку на плоскости для каждого пикселя и рисует
// анти-алиасинговые линии (minor/major) плюс две оси плоскости. Никаких
// ежекадровых загрузок вершинных буферов не производится.
// =================================================================================

#include "GridPass.h"

#include "../Core/RenderCommand.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"
#include "../Resources/Vertex.h"
#include "../OpenGL/OpenGLShader.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/VertexArray.h"
#include "MirEngine/Geometry/Scene/Scene.hpp"

#if defined(__APPLE__)
#    include <OpenGL/gl3.h>
#else
#    include <glad/gl.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace MirEngine::Rendering
{
namespace
{

constexpr char kGridVertSrc[] = R"GLSL(
#version 410 core
layout(location = 0) in vec3 aPos;
out vec2 v_ndc;
void main()
{
    v_ndc = aPos.xy;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
)GLSL";

constexpr char kGridFragSrc[] = R"GLSL(
#version 410 core
in vec2 v_ndc;
out vec4 FragColor;

uniform mat4 u_invViewProj;
uniform vec3 u_camPos;
uniform vec3 u_planeNormal;
uniform vec3 u_planePoint;
uniform vec3 u_uDir;
uniform vec3 u_vDir;
uniform float u_step;
uniform float u_fade;
uniform vec3 u_minorColor;
uniform vec3 u_majorColor;
uniform float u_axesEnabled;
uniform vec3 u_bgColor;

float gridLine(vec2 coord, float step)
{
    vec2 g = coord / step;
    vec2 d = abs(fract(g - 0.5) - 0.5) / fwidth(g);
    float line = min(d.x, d.y);
    return 1.0 - clamp(line, 0.0, 1.0);
}

float axisLine(float coord, float widthPx)
{
    float d = abs(coord);
    float fw = fwidth(coord);
    return 1.0 - clamp(d / (fw * widthPx), 0.0, 1.0);
}

void main()
{
    vec4 nearH = u_invViewProj * vec4(v_ndc, -1.0, 1.0);
    vec4 farH  = u_invViewProj * vec4(v_ndc,  1.0, 1.0);
    if (nearH.w == 0.0 || farH.w == 0.0) discard;
    vec3 nearP = nearH.xyz / nearH.w;
    vec3 farP  = farH.xyz / farH.w;

    vec3 ro = u_camPos;
    vec3 rd = farP - nearP;

    float denom = dot(rd, u_planeNormal);
    if (abs(denom) < 1e-7) discard;

    float t = dot(u_planePoint - ro, u_planeNormal) / denom;
    if (t <= 0.0) discard;

    vec3 hit = ro + rd * t;

    float dist = length(hit - ro);
    float fade = 1.0 - clamp(dist / max(u_fade, 1.0), 0.0, 1.0);
    if (fade <= 0.0) discard;

    vec2 coord = vec2(dot(hit - u_planePoint, u_uDir),
                      dot(hit - u_planePoint, u_vDir));

    float minor = gridLine(coord, u_step);
    float major = gridLine(coord, u_step * 5.0);
    vec3 color = mix(u_minorColor, u_majorColor, major);
    float intensity = max(minor, major);

    // Crisp in-plane axes through the plane origin:
    //   line coord.y = 0 runs along +X (red)
    //   line coord.x = 0 runs along +Z (blue)
    float axX = axisLine(coord.y, 1.5) * u_axesEnabled;
    float axZ = axisLine(coord.x, 1.5) * u_axesEnabled;
    float axis = max(axX, axZ);
    vec3 axisCol = axX * vec3(0.90, 0.22, 0.18)
                 + axZ * vec3(0.22, 0.34, 0.95);
    color = mix(color, axisCol, clamp(axis, 0.0, 1.0));
    intensity = max(intensity, axis);

    float a = intensity * fade;
    if (a <= 0.002) discard;
    FragColor = vec4(color, a);
}
)GLSL";

constexpr char kBgVertSrc[] = R"GLSL(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
out vec2 v_uv;
void main()
{
    v_uv = aUV;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
)GLSL";

constexpr char kBgFragSrc[] = R"GLSL(
#version 410 core
in vec2 v_uv;
out vec4 FragColor;
uniform vec3 u_topColor;
uniform vec3 u_bottomColor;
void main()
{
    float t = clamp(v_uv.y, 0.0, 1.0);
    FragColor = vec4(mix(u_bottomColor, u_topColor, t), 1.0);
}
)GLSL";

mir::Matrix4 rawToMatrix(const Matrix4Raw& r)
{
    mir::Matrix4 m;
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            m(row, c) = static_cast<double>(r[row + c * 4]);
    return m;
}

Matrix4Raw matrixToRaw(const mir::Matrix4& m)
{
    Matrix4Raw r{};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            r[row + c * 4] = static_cast<float>(m(row, c));
    return r;
}

} // namespace

GridPass::GridPass() = default;
GridPass::~GridPass() = default;

bool GridPass::createShaders()
{
    m_gridShader = std::make_unique<OpenGLShader>();
    if (!m_gridShader->compile(kGridVertSrc, kGridFragSrc))
    {
        std::cerr << "[GridPass] Failed to compile grid shader\n";
        return false;
    }

    m_bgShader = std::make_unique<OpenGLShader>();
    if (!m_bgShader->compile(kBgVertSrc, kBgFragSrc))
    {
        std::cerr << "[GridPass] Failed to compile background shader\n";
        return false;
    }
    return true;
}

void GridPass::buildBackground(RenderDevice& device)
{
    // Full-screen quad, static, built once.
    std::vector<Vertex> verts(6);
    verts[0].position = {-1.0f, -1.0f, 0.0f};
    verts[0].uv = {0.0f, 0.0f};
    verts[1].position = {1.0f, -1.0f, 0.0f};
    verts[1].uv = {1.0f, 0.0f};
    verts[2].position = {1.0f, 1.0f, 0.0f};
    verts[2].uv = {1.0f, 1.0f};
    verts[3].position = {-1.0f, -1.0f, 0.0f};
    verts[3].uv = {0.0f, 0.0f};
    verts[4].position = {1.0f, 1.0f, 0.0f};
    verts[4].uv = {1.0f, 1.0f};
    verts[5].position = {-1.0f, 1.0f, 0.0f};
    verts[5].uv = {0.0f, 1.0f};

    m_bgVBO = device.createVertexBuffer();
    m_bgVBO->uploadVertices(verts, BufferUsage::Static);
    m_bgIBO = device.createIndexBuffer();
    m_bgIBO->uploadIndices(std::vector<uint32_t>{0, 1, 2, 3, 4, 5},
                           BufferUsage::Static);
    m_bgVAO = device.createVertexArray();
    m_bgVAO->setVertexBuffer(m_bgVBO);
    m_bgVAO->setIndexBuffer(m_bgIBO);
}

void GridPass::buildGridQuad(RenderDevice& device)
{
    // Full-screen triangle (clip space). Only position is used by the shader.
    std::vector<Vertex> verts(3);
    verts[0].position = {-1.0f, -1.0f, 0.0f};
    verts[1].position = {3.0f, -1.0f, 0.0f};
    verts[2].position = {-1.0f, 3.0f, 0.0f};

    m_gridVBO = device.createVertexBuffer();
    m_gridVBO->uploadVertices(verts, BufferUsage::Static);
    m_gridIBO = device.createIndexBuffer();
    m_gridIBO->uploadIndices(std::vector<uint32_t>{0, 1, 2},
                             BufferUsage::Static);
    m_gridVAO = device.createVertexArray();
    m_gridVAO->setVertexBuffer(m_gridVBO);
    m_gridVAO->setIndexBuffer(m_gridIBO);
}

double GridPass::niceStep(double target) noexcept
{
    if (target <= 0.0) return 1.0;
    const double exp = std::floor(std::log10(target));
    const double base = std::pow(10.0, exp);
    const double f = target / base;
    double nice = 1.0;
    if (f >= 5.0) nice = 10.0;
    else if (f >= 2.0) nice = 5.0;
    else if (f >= 1.0) nice = 2.0;
    return nice * base;
}

void GridPass::planeAxes(GridPlane plane,
                         float normal[3],
                         float point[3],
                         float uDir[3],
                         float vDir[3]) noexcept
{
    std::memset(point, 0, 3 * sizeof(float));
    switch (plane)
    {
    case GridPlane::XY:
        normal[0] = 0.0f; normal[1] = 0.0f; normal[2] = 1.0f;
        uDir[0] = 1.0f; uDir[1] = 0.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 1.0f; vDir[2] = 0.0f;
        break;
    case GridPlane::YZ:
        normal[0] = 1.0f; normal[1] = 0.0f; normal[2] = 0.0f;
        uDir[0] = 0.0f; uDir[1] = 1.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 0.0f; vDir[2] = 1.0f;
        break;
    case GridPlane::XZ:
    default:
        normal[0] = 0.0f; normal[1] = 1.0f; normal[2] = 0.0f;
        uDir[0] = 1.0f; uDir[1] = 0.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 0.0f; vDir[2] = 1.0f;
        break;
    }
}

bool GridPass::initialize(RenderDevice& device)
{
    if (m_initialized) return true;
    if (!createShaders())
        return false;

    buildBackground(device);
    buildGridQuad(device);

    if (!m_bgVAO || !m_gridVAO)
    {
        std::cerr << "[GridPass] Failed to build static geometry\n";
        return false;
    }

    m_initialized = true;
    return true;
}

void GridPass::execute(RenderContext& context,
                       mir::Scene& /*scene*/,
                       RenderDevice& device)
{
    if (!m_initialized) return;

    // Overlay geometry is double-sided: disable back-face culling for the
    // full-screen background and grid passes.
    device.setCullFace(false);

    // Background gradient (static, opaque, drawn first).
    device.setDepthTest(false);
    device.setBlend(false);
    m_bgShader->bind();
    m_bgShader->setVec3("u_topColor", 0.16f, 0.18f, 0.22f);
    m_bgShader->setVec3("u_bottomColor", 0.04f, 0.05f, 0.07f);
    m_bgVAO->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    m_bgVAO->unbind();
    m_bgShader->unbind();

    if (!m_showGrid && !m_showAxes)
    {
        device.setCullFace(true);
        device.setDepthTest(true);
        return;
    }

    const mir::Matrix4 vp = rawToMatrix(context.projectionMatrix) *
                            rawToMatrix(context.viewMatrix);
    const mir::Matrix4 invVP = vp.inverse();
    const Matrix4Raw invRaw = matrixToRaw(invVP);

    float normal[3]{};
    float point[3]{};
    float uDir[3]{};
    float vDir[3]{};
    planeAxes(m_plane, normal, point, uDir, vDir);

    const float camPos[3] = {context.cameraPosition[0],
                             context.cameraPosition[1],
                             context.cameraPosition[2]};
    const double camDist = std::sqrt(
        std::pow(camPos[0] - point[0], 2.0) +
        std::pow(camPos[1] - point[1], 2.0) +
        std::pow(camPos[2] - point[2], 2.0));
    const float stepVal = static_cast<float>(niceStep(camDist * 0.1));

    float fade = m_fadeDistanceOverride;
    if (fade <= 0.0f)
        fade = std::max(context.farPlane * 0.6f, 40.0f);

    const float minorCol[3] = {m_showGrid ? 0.22f : 0.0f,
                                m_showGrid ? 0.24f : 0.0f,
                                m_showGrid ? 0.28f : 0.0f};
    const float majorCol[3] = {m_showGrid ? 0.40f : 0.0f,
                                m_showGrid ? 0.44f : 0.0f,
                                m_showGrid ? 0.52f : 0.0f};

    device.setDepthTest(false);
    device.setBlend(true);
    glDepthMask(GL_FALSE);

    m_gridShader->bind();
    m_gridShader->setMatrix("u_invViewProj", invRaw);
    m_gridShader->setVec3("u_camPos", camPos[0], camPos[1], camPos[2]);
    m_gridShader->setVec3("u_planeNormal", normal[0], normal[1], normal[2]);
    m_gridShader->setVec3("u_planePoint", point[0], point[1], point[2]);
    m_gridShader->setVec3("u_uDir", uDir[0], uDir[1], uDir[2]);
    m_gridShader->setVec3("u_vDir", vDir[0], vDir[1], vDir[2]);
    m_gridShader->setFloat("u_step", stepVal);
    m_gridShader->setFloat("u_fade", fade);
    m_gridShader->setVec3("u_minorColor", minorCol[0], minorCol[1], minorCol[2]);
    m_gridShader->setVec3("u_majorColor", majorCol[0], majorCol[1], majorCol[2]);
    m_gridShader->setFloat("u_axesEnabled", m_showAxes ? 1.0f : 0.0f);
    m_gridShader->setVec3("u_bgColor", 0.04f, 0.05f, 0.07f);

    m_gridVAO->bind();
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
    m_gridVAO->unbind();

    m_gridShader->unbind();

    glDepthMask(GL_TRUE);
    device.setDepthTest(true);
    device.setBlend(false);
    device.setCullFace(true);
}

} // namespace MirEngine::Rendering
