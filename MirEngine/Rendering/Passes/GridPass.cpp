#include "GridPass.h"
#include "../OpenGL/OpenGLShader.h"
#include "../OpenGL/OpenGLVertexArray.h"
#include "../OpenGL/OpenGLVertexBuffer.h"
#include "../OpenGL/OpenGLIndexBuffer.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"
#include "MirEngine/Math/TransformMatrix.hpp"

#include <algorithm>
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
namespace
{
constexpr float kQuadVertices[] = {
    -1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, 1.0f,
    -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, 1.0f,
};
constexpr unsigned int kQuadIndices[] = {0, 1, 2, 1, 3, 2};
}

static const char* kGridVS = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
void main() { gl_Position = vec4(aPos, 1.0); }
)";

static const char* kGridFS = R"(
#version 410 core
uniform mat4 uInvProjection;
uniform vec2 uScreenSize;
uniform vec3 uCamPos;
uniform vec3 uAnchor;
uniform vec3 uPlaneNormalView;
uniform vec3 uPlaneNormalWorld;
uniform float uPlaneConstView;
uniform mat3 uWorldFromView;
uniform float uMinorStep;
uniform float uMajorStep;
uniform float uFadeDistance;
uniform vec3 uMinorColor;
uniform vec3 uMajorColor;
uniform vec3 uAxisAColor;
uniform vec3 uAxisBColor;
uniform vec3 uVerticalColor;
out vec4 FragColor;

void main()
{
    vec2 ndc = gl_FragCoord.xy / max(uScreenSize, vec2(1.0)) * 2.0 - 1.0;
    vec4 farPoint = uInvProjection * vec4(ndc, 1.0, 1.0);
    vec3 dir = normalize(farPoint.xyz / max(abs(farPoint.w), 1e-8));

    float denom = dot(uPlaneNormalView, dir);
    if (abs(denom) < 1e-6) discard;
    float t = uPlaneConstView / denom;
    if (t < 0.0) discard;

    vec3 pView = dir * t;
    vec3 worldRelative = uWorldFromView * pView;
    vec3 local = worldRelative + (uCamPos - uAnchor);
    float dist = length(pView);
    float fade = 1.0 - smoothstep(uFadeDistance * 0.55, uFadeDistance, dist);
    if (fade <= 0.001) discard;

    // Coordinates in the active plane. XY uses X/Y, XZ uses X/Z, YZ uses Y/Z.
    vec2 coord;
    if (uPlaneNormalWorld.z > 0.5)
        coord = local.xy;
    else if (uPlaneNormalWorld.y > 0.5)
        coord = local.xz;
    else
        coord = local.yz;

    vec2 minorGrid = coord / max(uMinorStep, 1e-6);
    vec2 majorGrid = coord / max(uMajorStep, 1e-6);
    vec2 minorDist = abs(fract(minorGrid - 0.5) - 0.5) / max(fwidth(minorGrid), vec2(1e-6));
    vec2 majorDist = abs(fract(majorGrid - 0.5) - 0.5) / max(fwidth(majorGrid), vec2(1e-6));

    float minorLine = 1.0 - smoothstep(0.35, 1.0, min(minorDist.x, minorDist.y));
    float majorLine = 1.0 - smoothstep(0.35, 1.0, min(majorDist.x, majorDist.y));

    float axisWidth = max(fwidth(coord.x), fwidth(coord.y)) * 1.8;
    float axisA = 1.0 - smoothstep(axisWidth, axisWidth * 2.5, abs(coord.y));
    float axisB = 1.0 - smoothstep(axisWidth, axisWidth * 2.5, abs(coord.x));
    float extentA = 1.0 - smoothstep(uFadeDistance * 0.85, uFadeDistance, abs(coord.x));
    float extentB = 1.0 - smoothstep(uFadeDistance * 0.85, uFadeDistance, abs(coord.y));

    float originRadius = max(axisWidth * 3.0, 1e-5);
    float origin = 1.0 - smoothstep(originRadius, originRadius * 2.0,
                                     max(abs(coord.x), abs(coord.y)));

    vec3 color = mix(uMinorColor, uMajorColor, majorLine);
    float alpha = max(minorLine * 0.42, majorLine * 0.78);

    color = mix(color, uAxisAColor, axisA * extentA);
    alpha = max(alpha, axisA * extentA * 0.95);
    color = mix(color, uAxisBColor, axisB * extentB);
    alpha = max(alpha, axisB * extentB * 0.95);

    color = mix(color, uVerticalColor, origin);
    alpha = max(alpha, origin * 0.95);
    FragColor = vec4(color, alpha * fade);
}
)";

static const char* kAxisVS = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
uniform mat4 uVP;
uniform vec3 uAxisDirection;
uniform float uAxisLength;
uniform vec3 uCameraPosition;
out vec3 vLocal;
void main()
{
    vec3 worldPos = aPos * uAxisDirection * uAxisLength;
    vec3 cameraRelative = worldPos - uCameraPosition;
    vLocal = worldPos;
    gl_Position = uVP * vec4(cameraRelative, 1.0);
}
)";

static const char* kAxisFS = R"(
#version 410 core
in vec3 vLocal;
uniform vec3 uColor;
uniform float uFadeDistance;
out vec4 FragColor;
void main()
{
    float fade = 1.0 - smoothstep(uFadeDistance * 0.55, uFadeDistance, length(vLocal));
    if (fade <= 0.001) discard;
    FragColor = vec4(uColor, fade);
}
)";

GridPass::GridPass() = default;
GridPass::~GridPass() = default;

bool GridPass::initialize()
{
    if (m_initialized)
        return true;
    if (!createShaders())
    {
        std::cerr << "[GridPass] Failed to create shaders\n";
        return false;
    }
    buildQuad();
    buildAxis();
    m_initialized = true;
    return true;
}

bool GridPass::createShaders()
{
    m_gridShader = std::make_unique<OpenGLShader>();
    if (!m_gridShader->compile(kGridVS, kGridFS))
    {
        m_gridShader.reset();
        return false;
    }
    m_axisShader = std::make_unique<OpenGLShader>();
    if (!m_axisShader->compile(kAxisVS, kAxisFS))
    {
        m_axisShader.reset();
        return false;
    }
    return true;
}

void GridPass::buildQuad()
{
    std::vector<Vertex> vertices;
    for (int i = 0; i < 4; ++i)
    {
        vertices.push_back({{kQuadVertices[i * 3], kQuadVertices[i * 3 + 1], kQuadVertices[i * 3 + 2]},
                            {0, 1, 0}, {0, 0}});
    }
    std::vector<std::uint32_t> indices = {kQuadIndices[0], kQuadIndices[1], kQuadIndices[2],
                                          kQuadIndices[3], kQuadIndices[4], kQuadIndices[5]};
    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Static);
    auto ibo = std::make_shared<OpenGLIndexBuffer>();
    ibo->uploadIndices(indices, BufferUsage::Static);
    auto vao = std::make_shared<OpenGLVertexArray>();
    vao->setVertexBuffer(vbo);
    vao->setIndexBuffer(ibo);
    m_quadVBO = std::move(vbo);
    m_quadVAO = std::move(vao);
}

void GridPass::buildAxis()
{
    std::vector<Vertex> vertices = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}},
        {{1, 0, 0}, {0, 1, 0}, {0, 0}},
    };
    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Static);
    auto vao = std::make_shared<OpenGLVertexArray>();
    vao->setVertexBuffer(vbo);
    m_axisVBO = std::move(vbo);
    m_axisVAO = std::move(vao);
}

double GridPass::niceStep(double target) noexcept
{
    if (target <= 0.0)
        return 1.0;
    const double exponent = std::floor(std::log10(target));
    const double base = std::pow(10.0, exponent);
    const double mantissa = target / base;
    if (mantissa < 1.5) return base;
    if (mantissa < 3.5) return 2.0 * base;
    if (mantissa < 7.5) return 5.0 * base;
    return 10.0 * base;
}

void GridPass::planeBasis(GridPlane plane,
                          float normal[3],
                          float origin[3],
                          float axisAColor[3],
                          float axisBColor[3],
                          float verticalColor[3]) noexcept
{
    origin[0] = origin[1] = origin[2] = 0.0f;
    switch (plane)
    {
    case GridPlane::XY:
        normal[0] = 0.0f; normal[1] = 0.0f; normal[2] = 1.0f;
        axisAColor[0] = 0.92f; axisAColor[1] = 0.30f; axisAColor[2] = 0.30f;
        axisBColor[0] = 0.30f; axisBColor[1] = 0.85f; axisBColor[2] = 0.35f;
        verticalColor[0] = 0.28f; verticalColor[1] = 0.48f; verticalColor[2] = 0.95f;
        break;
    case GridPlane::XZ:
        normal[0] = 0.0f; normal[1] = 1.0f; normal[2] = 0.0f;
        axisAColor[0] = 0.92f; axisAColor[1] = 0.30f; axisAColor[2] = 0.30f;
        axisBColor[0] = 0.28f; axisBColor[1] = 0.48f; axisBColor[2] = 0.95f;
        verticalColor[0] = 0.30f; verticalColor[1] = 0.85f; verticalColor[2] = 0.35f;
        break;
    case GridPlane::YZ:
    default:
        normal[0] = 1.0f; normal[1] = 0.0f; normal[2] = 0.0f;
        axisAColor[0] = 0.30f; axisAColor[1] = 0.85f; axisAColor[2] = 0.35f;
        axisBColor[0] = 0.28f; axisBColor[1] = 0.48f; axisBColor[2] = 0.95f;
        verticalColor[0] = 0.92f; verticalColor[1] = 0.30f; verticalColor[2] = 0.30f;
        break;
    }
}

void GridPass::execute(RenderContext& context, mir::Scene&, RenderDevice&)
{
    if (!m_initialized || !m_gridShader || !m_axisShader || context.viewportWidth == 0 || context.viewportHeight == 0)
        return;

    float planeNormal[3]{};
    float planeOrigin[3]{};
    float axisAColor[3]{};
    float axisBColor[3]{};
    float verticalColor[3]{};
    planeBasis(m_plane, planeNormal, planeOrigin, axisAColor, axisBColor, verticalColor);

    const double eye[3] = {
        static_cast<double>(context.cameraPosition[0]),
        static_cast<double>(context.cameraPosition[1]),
        static_cast<double>(context.cameraPosition[2])};
    const double fov = std::max(static_cast<double>(context.fovY), 0.01);
    const double viewportHeight = static_cast<double>(context.viewportHeight);
    const double aspect = static_cast<double>(context.viewportWidth) / viewportHeight;

    double distPlane = std::abs(planeNormal[0] * (eye[0] - planeOrigin[0]) +
                                planeNormal[1] * (eye[1] - planeOrigin[1]) +
                                planeNormal[2] * (eye[2] - planeOrigin[2]));
    if (distPlane < 1e-5)
        distPlane = std::max(1.0, std::sqrt(eye[0]*eye[0] + eye[1]*eye[1] + eye[2]*eye[2]) * 0.02);

    const double worldHeight = 2.0 * distPlane * std::tan(fov * 0.5);
    const double worldWidth = worldHeight * aspect;
    double step = niceStep(worldHeight * 12.0 / viewportHeight);
    if (!std::isfinite(step) || step <= 0.0)
        step = 1.0;
    const double majorStep = step * 5.0;

    double anchor[3] = {0.0, 0.0, 0.0};
    if (m_plane == GridPlane::XY)
    {
        anchor[0] = std::floor(eye[0] / majorStep) * majorStep;
        anchor[1] = std::floor(eye[1] / majorStep) * majorStep;
    }
    else if (m_plane == GridPlane::XZ)
    {
        anchor[0] = std::floor(eye[0] / majorStep) * majorStep;
        anchor[2] = std::floor(eye[2] / majorStep) * majorStep;
    }
    else
    {
        anchor[1] = std::floor(eye[1] / majorStep) * majorStep;
        anchor[2] = std::floor(eye[2] / majorStep) * majorStep;
    }

    const float fade = std::max(0.01f,
        m_fadeDistanceOverride > 0.0f ? m_fadeDistanceOverride : context.farPlane * 0.85f);

    Matrix4Raw invProjection = IdentityMatrix4();
    mir::Matrix4 projection;
    const float* pm = context.projectionMatrix.data();
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            projection(row, col) = static_cast<mir::Scalar>(pm[row + col * 4]);
    const mir::Matrix4 inverse = projection.inverse();
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            invProjection[row + col * 4] = static_cast<float>(inverse(row, col));

    // View is column-major. Its upper-left 3x3 maps world -> view; the
    // transpose is the inverse rotation (view -> world).
    const float* v = context.viewMatrix.data();
    const float normalView[3] = {
        v[0] * planeNormal[0] + v[4] * planeNormal[1] + v[8] * planeNormal[2],
        v[1] * planeNormal[0] + v[5] * planeNormal[1] + v[9] * planeNormal[2],
        v[2] * planeNormal[0] + v[6] * planeNormal[1] + v[10] * planeNormal[2]};
    const double planeConst = planeNormal[0] * (planeOrigin[0] - eye[0]) +
                              planeNormal[1] * (planeOrigin[1] - eye[1]) +
                              planeNormal[2] * (planeOrigin[2] - eye[2]);
    const float worldFromView[9] = {
        v[0], v[4], v[8],
        v[1], v[5], v[9],
        v[2], v[6], v[10]};

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (m_showGrid && m_quadVAO)
    {
        m_gridShader->bind();
        m_gridShader->setMatrix("uInvProjection", invProjection);
        m_gridShader->setVec2("uScreenSize", static_cast<float>(context.viewportWidth), static_cast<float>(context.viewportHeight));
        m_gridShader->setVec3("uCamPos", context.cameraPosition[0], context.cameraPosition[1], context.cameraPosition[2]);
        m_gridShader->setVec3("uAnchor", static_cast<float>(anchor[0]), static_cast<float>(anchor[1]), static_cast<float>(anchor[2]));
        m_gridShader->setVec3("uPlaneNormalView", normalView[0], normalView[1], normalView[2]);
        m_gridShader->setVec3("uPlaneNormalWorld", planeNormal[0], planeNormal[1], planeNormal[2]);
        m_gridShader->setFloat("uPlaneConstView", static_cast<float>(planeConst));
        m_gridShader->setMatrix3("uWorldFromView", worldFromView);
        m_gridShader->setFloat("uMinorStep", static_cast<float>(step));
        m_gridShader->setFloat("uMajorStep", static_cast<float>(majorStep));
        m_gridShader->setFloat("uFadeDistance", fade);
        m_gridShader->setVec3("uMinorColor", 0.20f, 0.23f, 0.28f);
        m_gridShader->setVec3("uMajorColor", 0.36f, 0.40f, 0.48f);
        m_gridShader->setVec3("uAxisAColor", axisAColor[0], axisAColor[1], axisAColor[2]);
        m_gridShader->setVec3("uAxisBColor", axisBColor[0], axisBColor[1], axisBColor[2]);
        m_gridShader->setVec3("uVerticalColor", verticalColor[0], verticalColor[1], verticalColor[2]);
        m_quadVAO->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        m_quadVAO->unbind();
        m_gridShader->unbind();
    }

    if (m_showAxes && m_axisVAO)
    {
        const float axisLength = std::max(fade * 0.75f, static_cast<float>(std::max(worldWidth, worldHeight)));
        const float nx = m_plane == GridPlane::YZ ? 1.0f : 0.0f;
        const float ny = m_plane == GridPlane::XZ ? 1.0f : 0.0f;
        const float nz = m_plane == GridPlane::XY ? 1.0f : 0.0f;

        // Correct column-major P * R multiplication; translation is omitted
        // because the axis vertices are explicitly camera-relative.
        Matrix4Raw viewRel = context.viewMatrix;
        viewRel[12] = 0.0f;
        viewRel[13] = 0.0f;
        viewRel[14] = 0.0f;
        Matrix4Raw vpRel{};
        const float* p = context.projectionMatrix.data();
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                    sum += p[row + k * 4] * viewRel[k + col * 4];
                vpRel[row + col * 4] = sum;
            }

        m_axisShader->bind();
        m_axisShader->setMatrix("uVP", vpRel);
        m_axisShader->setVec3("uAxisDirection", nx, ny, nz);
        m_axisShader->setFloat("uAxisLength", axisLength);
        m_axisShader->setVec3("uCameraPosition", context.cameraPosition[0], context.cameraPosition[1], context.cameraPosition[2]);
        m_axisShader->setVec3("uColor", verticalColor[0], verticalColor[1], verticalColor[2]);
        m_axisShader->setFloat("uFadeDistance", fade);
        m_axisVAO->bind();
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, 2);
        m_axisVAO->unbind();
        glLineWidth(1.0f);
        m_axisShader->unbind();
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

} // namespace MirEngine::Rendering
