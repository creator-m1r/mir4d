#include "GridPass.h"
#include "../OpenGL/OpenGLShader.h"
#include "../OpenGL/OpenGLVertexArray.h"
#include "../OpenGL/OpenGLVertexBuffer.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"

#include <cmath>
#include <iostream>
#include <vector>

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

namespace MirEngine::Rendering
{

static const char* kGridVS = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
uniform mat4 uMVP;
uniform vec4 uColor;
out vec3 vPos;
out vec4 vColor;
void main() {
    vPos = aPos;
    vColor = uColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

static const char* kGridFS = R"(
#version 410 core
in vec3 vPos;
in vec4 vColor;
out vec4 FragColor;
uniform float uFadeDistance;
uniform vec3 uCameraPos;
void main() {
    float dist = length(vPos.xz - uCameraPos.xz);
    float alpha = 1.0 - smoothstep(uFadeDistance * 0.6, uFadeDistance, dist);
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
)";

GridPass::GridPass() = default;
GridPass::~GridPass() = default;

bool GridPass::initialize()
{
    if (m_initialized)
        return true;
    if (!createShader())
    {
        std::cerr << "[GridPass] Failed to create shader\n";
        return false;
    }
    buildGridGeometry();
    buildAxesGeometry();
    m_initialized = true;
    return true;
}

bool GridPass::createShader()
{
    m_shader = std::make_unique<OpenGLShader>();
    if (!m_shader->compile(kGridVS, kGridFS))
    {
        m_shader.reset();
        return false;
    }
    return true;
}

void GridPass::buildGridGeometry()
{
    std::vector<Vertex> vertices;
    const float half = m_gridSize;
    const float step = m_majorStep;
    const float minorStep = step / static_cast<float>(m_minorDivisions);

    for (float x = -half; x <= half + 0.001f; x += minorStep)
    {
        const bool isMajor = std::fmod(std::abs(x) + 0.0001f, step) < 0.001f;
        if (isMajor) continue;
        vertices.push_back({{x, 0.f, -half}, {0,1,0}, {0,0}});
        vertices.push_back({{x, 0.f, half}, {0,1,0}, {0,0}});
    }

    for (float z = -half; z <= half + 0.001f; z += minorStep)
    {
        const bool isMajor = std::fmod(std::abs(z) + 0.0001f, step) < 0.001f;
        if (isMajor) continue;
        vertices.push_back({{-half, 0.f, z}, {0,1,0}, {0,0}});
        vertices.push_back({{half, 0.f, z}, {0,1,0}, {0,0}});
    }

    for (float x = -half; x <= half + 0.001f; x += step)
    {
        vertices.push_back({{x, 0.f, -half}, {0,1,0}, {0,0}});
        vertices.push_back({{x, 0.f, half}, {0,1,0}, {0,0}});
    }

    for (float z = -half; z <= half + 0.001f; z += step)
    {
        vertices.push_back({{-half, 0.f, z}, {0,1,0}, {0,0}});
        vertices.push_back({{half, 0.f, z}, {0,1,0}, {0,0}});
    }

    m_gridVertexCount = static_cast<uint32_t>(vertices.size());
    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Static);
    auto vao = std::make_shared<OpenGLVertexArray>();
    vao->setVertexBuffer(vbo);
    m_gridVBO = std::move(vbo);
    m_gridVAO = std::move(vao);
}

void GridPass::buildAxesGeometry()
{
    const float len = m_gridSize * 0.6f;
    std::vector<Vertex> vertices = {
        {{0,0,0}, {0,1,0}, {0,0}}, {{len,0,0}, {0,1,0}, {0,0}},
        {{0,0,0}, {0,1,0}, {0,0}}, {{0,len,0}, {0,1,0}, {0,0}},
        {{0,0,0}, {0,1,0}, {0,0}}, {{0,0,len}, {0,1,0}, {0,0}}
    };

    m_axesVertexCount = static_cast<uint32_t>(vertices.size());
    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Static);
    auto vao = std::make_shared<OpenGLVertexArray>();
    vao->setVertexBuffer(vbo);
    m_axesVBO = std::move(vbo);
    m_axesVAO = std::move(vao);
}

void GridPass::execute(RenderContext& context,
                       mir::Scene&,
                       RenderDevice&)
{
    if (!m_initialized || !m_shader)
        return;

    m_shader->bind();
    m_shader->setMatrix("uMVP", context.viewProjectionMatrix);
    m_shader->setFloat("uFadeDistance", m_fadeDistance);
    m_shader->setVec3("uCameraPos", context.cameraPosition[0], context.cameraPosition[1], context.cameraPosition[2]);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    if (m_showGrid && m_gridVAO)
    {
        m_shader->setVec4("uColor", 0.22f, 0.26f, 0.32f, 0.7f);
        m_gridVAO->bind();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_gridVertexCount));
        m_gridVAO->unbind();
    }

    if (m_showAxes && m_axesVAO)
    {
        m_axesVAO->bind();
        m_shader->setVec4("uColor", 0.92f, 0.25f, 0.25f, 1.0f);
        glDrawArrays(GL_LINES, 0, 2);
        m_shader->setVec4("uColor", 0.25f, 0.85f, 0.35f, 1.0f);
        glDrawArrays(GL_LINES, 2, 2);
        m_shader->setVec4("uColor", 0.25f, 0.45f, 0.95f, 1.0f);
        glDrawArrays(GL_LINES, 4, 2);
        m_axesVAO->unbind();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    m_shader->unbind();
}

} // namespace MirEngine::Rendering
