// MirEngine/Rendering/Passes/GridPass.cpp
// =================================================================================
// Реализация GridPass — сетка + оси для CAD-вьюпорта.
// =================================================================================

#include "GridPass.h"
#include "../OpenGL/OpenGLShader.h"
#include "../OpenGL/OpenGLVertexArray.h"
#include "../OpenGL/OpenGLVertexBuffer.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace MirEngine {
namespace Rendering {

// --------------------------------------------------------------------------
// Простой шейдер сетки (position + color)
// --------------------------------------------------------------------------
static const char* kGridVS = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;   // не используется
layout(location = 2) in vec2 aUV;       // не используется
layout(location = 3) in vec4 aColor;    // если расширим Vertex позже

uniform mat4 uMVP;
out vec3 vPos;
out vec4 vColor;

// Временный вариант: цвет передаём через uniform или хардкодим в VS по оси
uniform vec4 uColor;

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
uniform vec3  uCameraPos;

void main() {
    float dist = length(vPos.xz - uCameraPos.xz); // затухание по XZ
    float alpha = 1.0 - smoothstep(uFadeDistance * 0.6, uFadeDistance, dist);
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
)";

// --------------------------------------------------------------------------
// Конструктор / деструктор
// --------------------------------------------------------------------------
GridPass::GridPass() = default;

GridPass::~GridPass() = default;

// --------------------------------------------------------------------------
// Инициализация
// --------------------------------------------------------------------------
bool GridPass::initialize() {
    if (m_initialized) return true;

    if (!createShader()) {
        std::cerr << "[GridPass] Failed to create shader\n";
        return false;
    }

    buildGridGeometry();
    buildAxesGeometry();

    m_initialized = true;
    std::cout << "[GridPass] Initialized (grid + axes)\n";
    return true;
}

// --------------------------------------------------------------------------
// Шейдер
// --------------------------------------------------------------------------
bool GridPass::createShader() {
    m_shader = std::make_unique<OpenGLShader>();
    if (!m_shader->compile(kGridVS, kGridFS)) {
        m_shader.reset();
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------
// Геометрия сетки (линии в плоскости XZ, Y = 0)
// --------------------------------------------------------------------------
void GridPass::buildGridGeometry() {
    std::vector<Vertex> vertices;

    const float half = m_gridSize;
    const float step = m_majorStep;
    const float minorStep = step / static_cast<float>(m_minorDivisions);

    // --- Мелкие линии ---
    for (float x = -half; x <= half + 0.001f; x += minorStep) {
        // пропускаем линии, совпадающие с major (они будут ярче)
        bool isMajor = std::fmod(std::abs(x) + 0.0001f, step) < 0.001f;
        if (isMajor) continue;

        vertices.push_back({ {x, 0.f, -half}, {0,1,0}, {0,0} });
        vertices.push_back({ {x, 0.f,  half}, {0,1,0}, {0,0} });
    }
    for (float z = -half; z <= half + 0.001f; z += minorStep) {
        bool isMajor = std::fmod(std::abs(z) + 0.0001f, step) < 0.001f;
        if (isMajor) continue;

        vertices.push_back({ {-half, 0.f, z}, {0,1,0}, {0,0} });
        vertices.push_back({ { half, 0.f, z}, {0,1,0}, {0,0} });
    }

    // --- Основные линии ---
    for (float x = -half; x <= half + 0.001f; x += step) {
        vertices.push_back({ {x, 0.f, -half}, {0,1,0}, {0,0} });
        vertices.push_back({ {x, 0.f,  half}, {0,1,0}, {0,0} });
    }
    for (float z = -half; z <= half + 0.001f; z += step) {
        vertices.push_back({ {-half, 0.f, z}, {0,1,0}, {0,0} });
        vertices.push_back({ { half, 0.f, z}, {0,1,0}, {0,0} });
    }

    m_gridVertexCount = static_cast<uint32_t>(vertices.size());

    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Static);

    auto vao = std::make_shared<OpenGLVertexArray>();
    vao->setVertexBuffer(vbo);

    m_gridVBO = vbo;
    m_gridVAO = vao;
}

// --------------------------------------------------------------------------
// Оси (X — красный, Y — зелёный, Z — синий)
// --------------------------------------------------------------------------
void GridPass::buildAxesGeometry() {
    const float len = m_gridSize * 0.6f;

    std::vector<Vertex> vertices = {
        // X axis
        { {0, 0, 0}, {0,1,0}, {0,0} },
        { {len, 0, 0}, {0,1,0}, {0,0} },
        // Y axis
        { {0, 0, 0}, {0,1,0}, {0,0} },
        { {0, len, 0}, {0,1,0}, {0,0} },
        // Z axis
        { {0, 0, 0}, {0,1,0}, {0,0} },
        { {0, 0, len}, {0,1,0}, {0,0} }
    };

    m_axesVertexCount = static_cast<uint32_t>(vertices.size());

    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Static);

    auto vao = std::make_shared<OpenGLVertexArray>();
    vao->setVertexBuffer(vbo);

    m_axesVBO = vbo;
    m_axesVAO = vao;
}

// --------------------------------------------------------------------------
// Выполнение прохода
// --------------------------------------------------------------------------
void GridPass::execute(RenderContext& context,
                       Scene& /*scene*/,
                       Camera& /*camera*/,
                       RenderDevice& /*device*/)
{
    if (!m_initialized || !m_shader) return;

    // Собираем MVP = Projection * View * Model(identity)
    Matrix4Raw mvp = context.viewProjectionMatrix;

    m_shader->bind();
    m_shader->setMatrix("uMVP", mvp);
    m_shader->setFloat("uFadeDistance", m_fadeDistance);
    m_shader->setVec3("uCameraPos",
                      context.cameraPosition[0],
                      context.cameraPosition[1],
                      context.cameraPosition[2]);

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <glad/gl.h>
#endif

    // Отключаем запись в depth для сетки (или оставляем — на вкус)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);          // сетка не пишет в depth
    glDisable(GL_CULL_FACE);

    // ---- Мелкая + основная сетка ----
    if (m_showGrid && m_gridVAO) {
        // Сначала мелкие линии (более тёмные)
        // Для простоты рисуем всё одним цветом, major можно выделить вторым проходом
        m_shader->setVec4("uColor", 0.22f, 0.26f, 0.32f, 0.7f);

        m_gridVAO->bind();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_gridVertexCount));
        m_gridVAO->unbind();
    }

    // ---- Оси ----
    if (m_showAxes && m_axesVAO) {
        m_axesVAO->bind();

        // X — красный
        m_shader->setVec4("uColor", 0.92f, 0.25f, 0.25f, 1.0f);
        glDrawArrays(GL_LINES, 0, 2);

        // Y — зелёный
        m_shader->setVec4("uColor", 0.25f, 0.85f, 0.35f, 1.0f);
        glDrawArrays(GL_LINES, 2, 2);

        // Z — синий
        m_shader->setVec4("uColor", 0.25f, 0.45f, 0.95f, 1.0f);
        glDrawArrays(GL_LINES, 4, 2);

        m_axesVAO->unbind();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    m_shader->unbind();
}

} // namespace Rendering
} // namespace MirEngine