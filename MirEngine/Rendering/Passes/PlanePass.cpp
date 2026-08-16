// MirEngine/Rendering/Passes/PlanePass.cpp
// =================================================================================
// Оверлей рабочих плоскостей (ТЗ Этап 1, раздел 5).
// =================================================================================

#include "PlanePass.h"

#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"
#include "../Resources/Vertex.h"
#include "../OpenGL/OpenGLShader.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/VertexArray.h"
#include "MirEngine/Math/TransformMatrix.hpp"
#include "MirEngine/Math/Vector/Vector3.hpp"

#if defined(__APPLE__)
#    include <OpenGL/gl3.h>
#else
#    include <glad/gl.h>
#endif

#include <iostream>
#include <cmath>

namespace MirEngine::Rendering
{
namespace
{

constexpr char kVertSrc[] = R"GLSL(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
uniform mat4 u_viewProj;
out vec4 v_color;
void main()
{
    // color packed as (normal.rgb, uv.x = alpha)
    v_color = vec4(aNormal, aUV.x);
    gl_Position = u_viewProj * vec4(aPos, 1.0);
}
)GLSL";

constexpr char kFragSrc[] = R"GLSL(
#version 410 core
in vec4 v_color;
out vec4 FragColor;
void main()
{
    FragColor = v_color;
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

float niceStepLocal(double target)
{
    if (target <= 0.0) return 1.0;
    const double exp = std::floor(std::log10(target));
    const double base = std::pow(10.0, exp);
    const double f = target / base;
    double nice = 1.0;
    if (f >= 5.0) nice = 10.0;
    else if (f >= 2.0) nice = 5.0;
    else if (f >= 1.0) nice = 2.0;
    return static_cast<float>(nice * base);
}

// Половинный размер плоскости адаптируется под зум камеры, чтобы плоскости
// визуально совмещались с адаптивной сеткой GridPass (ТЗ Этап 1, отображение).
float adaptivePlaneHalfExtent(const RenderContext& context)
{
    const float* cp = context.cameraPosition;
    const double camDist = std::sqrt(double(cp[0]) * cp[0] + double(cp[1]) * cp[1] + double(cp[2]) * cp[2]);
    const float stepVal = niceStepLocal(camDist * 0.1);
    // Ограничиваем половинный размер: плоскость должна быть заметной, но не
    // закрывать весь вид при удалённой камере (иначе после клиппинга —
    // "лепестки").
    float s = stepVal;
    s = std::min(s, 60.0f);
    s = std::max(s, 8.0f);
    return s;
}

} // namespace

PlanePass::PlanePass() = default;
PlanePass::~PlanePass() = default;

bool PlanePass::createShaders()
{
    m_shader = std::make_unique<OpenGLShader>();
    if (!m_shader->compile(kVertSrc, kFragSrc))
    {
        std::cerr << "[PlanePass] Failed to compile shader\n";
        return false;
    }
    return true;
}

void PlanePass::buildDynamicGeometry(RenderDevice& device,
                                     const std::vector<PlaneRenderData>& planes,
                                     float sizeScale)
{
    std::vector<Vertex> verts;
    std::vector<uint32_t> indices;
    verts.reserve(planes.size() * 40);
    indices.reserve(planes.size() * 28);

    const auto push = [&](float x, float y, float z,
                          float r, float g, float b, float a)
    {
        Vertex v;
        v.position = {x, y, z};
        v.normal = {r, g, b};
        v.uv = {a, 0.0f};
        verts.push_back(v);
    };

    // Индексы группируются ПО ТИПАМ примитивов: сначала все треугольники
    // поверхностей, затем все линии (оси + рамки). Иначе glDrawElements для
    // треугольников и линий читают пересекающиеся диапазоны индексов, и вершины
    // осей/рамок попадают в треугольники -> "иглы"/"клинья".

    // Pass 1: поверхности (2 треугольника на плоскость).
    {
        uint32_t base = 0;
        for (const auto& p : planes)
        {
            const mir::Vector3 o{p.origin[0], p.origin[1], p.origin[2]};
            const mir::Vector3 nx = mir::Vector3{p.xAxis[0], p.xAxis[1], p.xAxis[2]}.normalized();
            const mir::Vector3 ny = mir::Vector3{p.yAxis[0], p.yAxis[1], p.yAxis[2]}.normalized();
            const float s = sizeScale;
            const float surfaceA = p.active ? 0.16f : (p.selected ? 0.12f : 0.05f);
            const float r = p.color[0], g = p.color[1], b = p.color[2];

            const mir::Vector3 c00 = o - nx * s - ny * s;
            const mir::Vector3 c10 = o + nx * s - ny * s;
            const mir::Vector3 c11 = o + nx * s + ny * s;
            const mir::Vector3 c01 = o - nx * s + ny * s;
            push(c00.x, c00.y, c00.z, r, g, b, surfaceA);
            push(c10.x, c10.y, c10.z, r, g, b, surfaceA);
            push(c11.x, c11.y, c11.z, r, g, b, surfaceA);
            push(c01.x, c01.y, c01.z, r, g, b, surfaceA);
            indices.push_back(base + 0); indices.push_back(base + 1); indices.push_back(base + 2);
            indices.push_back(base + 0); indices.push_back(base + 2); indices.push_back(base + 3);
            base += 4;
        }
    }

    // Pass 2: оси (X красная, Y зелёная, нормаль синяя) — 3 отрезка = 6 вершин.
    {
        uint32_t base = static_cast<uint32_t>(verts.size());
        for (const auto& p : planes)
        {
            const mir::Vector3 o{p.origin[0], p.origin[1], p.origin[2]};
            const mir::Vector3 nx = mir::Vector3{p.xAxis[0], p.xAxis[1], p.xAxis[2]}.normalized();
            const mir::Vector3 ny = mir::Vector3{p.yAxis[0], p.yAxis[1], p.yAxis[2]}.normalized();
            const mir::Vector3 n = mir::Vector3{p.normal[0], p.normal[1], p.normal[2]}.normalized();
            const float s = sizeScale;
            const float axisLen = s * 1.1f;
            const float nLen = s * 0.5f;
            const mir::Vector3 ox = o + nx * axisLen;
            const mir::Vector3 oy = o + ny * axisLen;
            const mir::Vector3 on = o + n * nLen;
            push(o.x, o.y, o.z, 0.90f, 0.22f, 0.18f, 1.0f);
            push(ox.x, ox.y, ox.z, 0.90f, 0.22f, 0.18f, 1.0f);
            push(o.x, o.y, o.z, 0.20f, 0.70f, 0.30f, 1.0f);
            push(oy.x, oy.y, oy.z, 0.20f, 0.70f, 0.30f, 1.0f);
            push(o.x, o.y, o.z, 0.22f, 0.34f, 0.95f, 1.0f);
            push(on.x, on.y, on.z, 0.22f, 0.34f, 0.95f, 1.0f);
            indices.push_back(base + 0); indices.push_back(base + 1);
            indices.push_back(base + 2); indices.push_back(base + 3);
            indices.push_back(base + 4); indices.push_back(base + 5);
            base += 6;
        }
    }

    // Pass 3: рамки плоскостей (всегда, активная/выбранная — ярче).
    {
        uint32_t base = static_cast<uint32_t>(verts.size());
        for (const auto& p : planes)
        {
            const mir::Vector3 o{p.origin[0], p.origin[1], p.origin[2]};
            const mir::Vector3 nx = mir::Vector3{p.xAxis[0], p.xAxis[1], p.xAxis[2]}.normalized();
            const mir::Vector3 ny = mir::Vector3{p.yAxis[0], p.yAxis[1], p.yAxis[2]}.normalized();
            const float s = sizeScale;
            const float r = p.color[0], g = p.color[1], b = p.color[2];
            const float borderA = p.active ? 1.0f : (p.selected ? 0.95f : 0.7f);
            const mir::Vector3 c00 = o - nx * s - ny * s;
            const mir::Vector3 c10 = o + nx * s - ny * s;
            const mir::Vector3 c11 = o + nx * s + ny * s;
            const mir::Vector3 c01 = o - nx * s + ny * s;
            push(c00.x, c00.y, c00.z, r, g, b, borderA);
            push(c10.x, c10.y, c10.z, r, g, b, borderA);
            push(c10.x, c10.y, c10.z, r, g, b, borderA);
            push(c11.x, c11.y, c11.z, r, g, b, borderA);
            push(c11.x, c11.y, c11.z, r, g, b, borderA);
            push(c01.x, c01.y, c01.z, r, g, b, borderA);
            push(c01.x, c01.y, c01.z, r, g, b, borderA);
            push(c00.x, c00.y, c00.z, r, g, b, borderA);
            indices.push_back(base + 0); indices.push_back(base + 1);
            indices.push_back(base + 2); indices.push_back(base + 3);
            indices.push_back(base + 4); indices.push_back(base + 5);
            indices.push_back(base + 6); indices.push_back(base + 7);
            base += 8;
        }
    }

    if (verts.empty())
        return;

    if (!m_vbo)
    {
        m_vbo = device.createVertexBuffer();
        m_ibo = device.createIndexBuffer();
        m_vao = device.createVertexArray();
    }
    m_vbo->uploadVertices(verts, BufferUsage::Static);
    m_ibo->uploadIndices(indices, BufferUsage::Static);
    m_vao->setVertexBuffer(m_vbo);
    m_vao->setIndexBuffer(m_ibo);
}

bool PlanePass::initialize(RenderDevice& device)
{
    if (m_initialized)
        return true;
    if (!createShaders())
        return false;
    m_initialized = true;
    return true;
}

void PlanePass::execute(RenderContext& context,
                        mir::Scene& /*scene*/,
                        RenderDevice& device)
{
    if (!m_initialized || context.planes.empty())
        return;

    buildDynamicGeometry(device, context.planes, adaptivePlaneHalfExtent(context));

    const mir::Matrix4 vp = rawToMatrix(context.viewProjectionMatrix);
    const Matrix4Raw vpRaw = matrixToRaw(vp);

    device.setCullFace(false);
    device.setDepthTest(true);
    device.setBlend(true);
    glDepthMask(GL_FALSE);

    m_shader->bind();
    m_shader->setMatrix("u_viewProj", vpRaw);
    m_vao->bind();

    const std::uint32_t triCount = static_cast<std::uint32_t>(context.planes.size()) * 2;
    glDrawElements(GL_TRIANGLES, triCount * 3, GL_UNSIGNED_INT, nullptr);

    const std::uint32_t triIndices = triCount * 3;
    const std::uint32_t lineTotal = static_cast<std::uint32_t>(context.planes.size()) * (6 + 8);
    if (lineTotal > 0)
    {
        const auto* offset = reinterpret_cast<const void*>(
            static_cast<std::uintptr_t>(triIndices * sizeof(uint32_t)));
        glDrawElements(GL_LINES, lineTotal, GL_UNSIGNED_INT, offset);
    }

    m_vao->unbind();
    m_shader->unbind();

    glDepthMask(GL_TRUE);
    device.setDepthTest(true);
    device.setBlend(false);
    device.setCullFace(true);
}

} // namespace MirEngine::Rendering
