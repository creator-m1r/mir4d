// MirEngine/Rendering/Passes/SketchPass.cpp
// =================================================================================
// Оверлей 2D-эскиза на рабочей плоскости (ТЗ Этап 2).
// =================================================================================

#include "SketchPass.h"

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

} // namespace

SketchPass::SketchPass() = default;
SketchPass::~SketchPass() = default;

bool SketchPass::createShaders()
{
    m_shader = std::make_unique<OpenGLShader>();
    if (!m_shader->compile(kVertSrc, kFragSrc))
    {
        std::cerr << "[SketchPass] Failed to compile shader\n";
        return false;
    }
    return true;
}

void SketchPass::buildDynamicGeometry(RenderDevice& device,
                                     const std::vector<SketchRenderData>& sketches)
{
    std::vector<Vertex> verts;
    std::vector<uint32_t> indices;

    uint32_t base = 0;
    for (const auto& sk : sketches)
    {
        const mir::Vector3 o{sk.origin[0], sk.origin[1], sk.origin[2]};
        const mir::Vector3 nx{sk.xAxis[0], sk.xAxis[1], sk.xAxis[2]};
        const mir::Vector3 ny{sk.yAxis[0], sk.yAxis[1], sk.yAxis[2]};

        for (const auto& seg : sk.segments)
        {
            const mir::Vector3 a = o + nx * seg.ax + ny * seg.ay;
            const mir::Vector3 b = o + nx * seg.bx + ny * seg.by;
            Vertex va;
            va.position = {static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z)};
            va.normal = {seg.color[0], seg.color[1], seg.color[2]};
            va.uv = {1.0f, 0.0f};
            Vertex vb = va;
            vb.position = {static_cast<float>(b.x), static_cast<float>(b.y), static_cast<float>(b.z)};
            verts.push_back(va);
            verts.push_back(vb);
            indices.push_back(base);
            indices.push_back(base + 1);
            base += 2;
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

bool SketchPass::initialize(RenderDevice& device)
{
    if (m_initialized)
        return true;
    if (!createShaders())
        return false;
    m_initialized = true;
    return true;
}

void SketchPass::execute(RenderContext& context,
                        mir::Scene& /*scene*/,
                        RenderDevice& device)
{
    if (!m_initialized || context.sketches.empty())
        return;

    buildDynamicGeometry(device, context.sketches);

    const mir::Matrix4 vp = rawToMatrix(context.viewProjectionMatrix);
    const Matrix4Raw vpRaw = matrixToRaw(vp);

    device.setCullFace(false);
    device.setDepthTest(true);
    device.setBlend(true);
    glDepthMask(GL_FALSE);

    m_shader->bind();
    m_shader->setMatrix("u_viewProj", vpRaw);
    m_vao->bind();

    const std::uint32_t lineCount =
        static_cast<std::uint32_t>(context.sketches.size() ? context.sketches[0].segments.size() : 0);
    if (lineCount > 0)
        glDrawElements(GL_LINES, lineCount * 2, GL_UNSIGNED_INT, nullptr);

    m_vao->unbind();
    m_shader->unbind();

    glDepthMask(GL_TRUE);
    device.setDepthTest(true);
    device.setBlend(false);
    device.setCullFace(true);
}

} // namespace MirEngine::Rendering
