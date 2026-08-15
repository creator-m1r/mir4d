#include "GridPass.h"
#include "../OpenGL/OpenGLShader.h"
#include "../OpenGL/OpenGLVertexArray.h"
#include "../OpenGL/OpenGLVertexBuffer.h"
#include "../OpenGL/OpenGLIndexBuffer.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"
#include "MirEngine/Math/TransformMatrix.hpp"

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

// Full-screen quad in NDC. The fragment shader reconstructs the view-space
// ray from the inverse projection, so the grid is truly infinite.
constexpr float kQuadVertices[] = {
    -1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, 1.0f,
    -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, 1.0f,
};

constexpr unsigned int kQuadIndices[] = {0, 1, 2, 1, 3, 2};

} // namespace

static const char* kGridVS = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// Infinite procedural grid:
//   ray (view space) -> plane intersection -> camera-relative local coords
//   -> minor/major lines, workspace axes, origin marker, distance fade.
static const char* kGridFS = R"(
#version 410 core
uniform mat4 uInvProjection;
uniform vec2 uScreenSize;
uniform vec3 uCamPos;          // camera position (world)
uniform vec3 uAnchor;          // grid anchor (world, snapped CPU-side)
uniform vec3 uPlaneNormalView; // plane normal expressed in view space
uniform vec3 uPlaneNormalWorld; // plane normal expressed in world space
uniform float uPlaneConstView; // n_v . p_v = uPlaneConstView
uniform mat3 uWorldFromView;   // view-space -> world-space rotation (R^T)
uniform float uMinorStep;
uniform float uMajorStep;
uniform float uFadeDistance;
uniform float uCameraHeight;    // perpendicular camera-to-plane distance
uniform vec3 uMinorColor;
uniform vec3 uMajorColor;
uniform vec3 uAxisAColor;
uniform vec3 uAxisBColor;
uniform vec3 uVerticalColor;

out vec4 FragColor;

void main() {
    vec2 ndc = gl_FragCoord.xy / uScreenSize * 2.0 - 1.0;
    vec4 farPoint = uInvProjection * vec4(ndc, 1.0, 1.0);
    vec3 dir = normalize(farPoint.xyz / farPoint.w);

    float denom = dot(uPlaneNormalView, dir);
    if (abs(denom) < 1e-6) discard;           // plane edge-on to the camera
    // Note: for an orbit camera above the plane the normal points away from
    // the viewer, so denom is negative while planeConst is negative too and
    // t = planeConst / denom is positive (the plane is in front). Only the
    // signed ray-plane test below decides visibility.

    float t = uPlaneConstView / denom;
    if (t < 0.0) discard;

    vec3 pView = dir * t;
    vec3 local = uWorldFromView * pView + (uCamPos - uAnchor);

    vec2 c1 = (uPlaneNormalWorld.z > 0.5) ? local.xz
           : ((uPlaneNormalWorld.y > 0.5) ? local.xy : local.yz);

    // Fade by the distance measured ALONG the plane (uniform circles around
    // the camera's projection onto the plane), not by the 3D eye distance:
    // rays near the horizon travel long before hitting the plane, which made
    // the side regions of the viewport fade out while the center stayed dark.
    // A radius of 5 * cameraHeight covers the whole viewport: the steepest
    // side rays hit the plane at ~2.7 * cameraHeight, so they stay well
    // inside the fade band.
    float distPlane = length(pView);
    float distAlong = sqrt(max(0.0, distPlane * distPlane - uCameraHeight * uCameraHeight));
    float fade = 1.0 - smoothstep(uFadeDistance * 0.55, uFadeDistance, distAlong);
    if (fade <= 0.0) discard;

    // Minor lines
    vec2 gMinor = c1 / uMinorStep;
    vec2 dMinor = abs(fract(gMinor - 0.5) - 0.5) / fwidth(gMinor);
    float minorLine = 1.0 - smoothstep(0.5, 1.0, min(dMinor.x, dMinor.y));

    // Major lines
    vec2 gMajor = c1 / uMajorStep;
    vec2 dMajor = abs(fract(gMajor - 0.5) - 0.5) / fwidth(gMajor);
    float majorLine = 1.0 - smoothstep(0.5, 1.0, min(dMajor.x, dMajor.y));

    // Workspace axes on the plane: first component == 0, second component == 0
    float fwA = fwidth(local.x) + fwidth(local.y) + fwidth(local.z);
    float axisA = 1.0 - smoothstep(0.5, 1.5, abs(c1.y) / max(fwA, 1e-6));
    float axisB = 1.0 - smoothstep(0.5, 1.5, abs(c1.x) / max(fwA, 1e-6));
    float axisAExtent = smoothstep(0.0, 1.0, 1.0 - abs(c1.x) / (uFadeDistance * 0.85));
    float axisBExtent = smoothstep(0.0, 1.0, 1.0 - abs(c1.y) / (uFadeDistance * 0.85));

    // Origin marker (small cross)
    float origin = 1.0 - smoothstep(0.5, 2.0, max(abs(c1.x), abs(c1.y)) / max(fwA, 1e-6));

    vec3 color = mix(uMinorColor, uMajorColor, majorLine);
    float alpha = max(minorLine * 0.45, majorLine * 0.9);

    color = mix(color, uAxisAColor, axisA * axisAExtent);
    alpha = max(alpha, axisA * axisAExtent * 0.95);
    color = mix(color, uAxisBColor, axisB * axisBExtent);
    alpha = max(alpha, axisB * axisBExtent * 0.95);

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
out vec3 vLocal;
void main() {
    vec3 p = aPos * uAxisDirection * uAxisLength;
    vLocal = p;
    gl_Position = uVP * vec4(p, 1.0);
}
)";

static const char* kAxisFS = R"(
#version 410 core
in vec3 vLocal;
uniform vec3 uColor;
uniform float uFadeDistance;
out vec4 FragColor;
void main() {
    float dist = length(vLocal);
    float fade = 1.0 - smoothstep(uFadeDistance * 0.55, uFadeDistance, dist);
    if (fade <= 0.0) discard;
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
    // Unit-direction axis line; direction and length are applied in the shader.
    std::vector<Vertex> vertices = {
        {{0, 0, 0}, {0, 1, 0}, {0, 0}},
        {{1, 0, 0}, {0, 1, 0}, {0, 0}},
    };

    auto vbo = std::make_shared<OpenGLVertexBuffer>();
    vbo->uploadVertices(vertices, BufferUsage::Dynamic);
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
    if (mantissa < 1.5) return 1.0 * base;
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
    switch (plane)
    {
    case GridPlane::XY:
        normal[0] = 0.0f; normal[1] = 0.0f; normal[2] = 1.0f;
        origin[0] = 0.0f; origin[1] = 0.0f; origin[2] = 0.0f;
        axisAColor[0] = 0.92f; axisAColor[1] = 0.30f; axisAColor[2] = 0.30f; // X red
        axisBColor[0] = 0.30f; axisBColor[1] = 0.85f; axisBColor[2] = 0.35f; // Y green
        verticalColor[0] = 0.28f; verticalColor[1] = 0.48f; verticalColor[2] = 0.95f; // Z blue
        break;
    case GridPlane::XZ:
        normal[0] = 0.0f; normal[1] = 1.0f; normal[2] = 0.0f;
        origin[0] = 0.0f; origin[1] = 0.0f; origin[2] = 0.0f;
        axisAColor[0] = 0.92f; axisAColor[1] = 0.30f; axisAColor[2] = 0.30f; // X red
        axisBColor[0] = 0.28f; axisBColor[1] = 0.48f; axisBColor[2] = 0.95f; // Z blue
        verticalColor[0] = 0.30f; verticalColor[1] = 0.85f; verticalColor[2] = 0.35f; // Y green
        break;
    case GridPlane::YZ:
    default:
        normal[0] = 1.0f; normal[1] = 0.0f; normal[2] = 0.0f;
        origin[0] = 0.0f; origin[1] = 0.0f; origin[2] = 0.0f;
        axisAColor[0] = 0.30f; axisAColor[1] = 0.85f; axisAColor[2] = 0.35f; // Y green
        axisBColor[0] = 0.28f; axisBColor[1] = 0.48f; axisBColor[2] = 0.95f; // Z blue
        verticalColor[0] = 0.92f; verticalColor[1] = 0.30f; verticalColor[2] = 0.30f; // X red
        break;
    }
}

void GridPass::execute(RenderContext& context,
                       mir::Scene&,
                       RenderDevice&)
{
    if (!m_initialized || !m_gridShader || !m_axisShader)
        return;

    float planeNormal[3] = {0, 0, 1};
    float planeOrigin[3] = {0, 0, 0};
    float axisAColor[3] = {0.92f, 0.30f, 0.30f};
    float axisBColor[3] = {0.30f, 0.85f, 0.35f};
    float verticalColor[3] = {0.28f, 0.48f, 0.95f};
    planeBasis(m_plane, planeNormal, planeOrigin, axisAColor, axisBColor, verticalColor);

    const double eyeX = static_cast<double>(context.cameraPosition[0]);
    const double eyeY = static_cast<double>(context.cameraPosition[1]);
    const double eyeZ = static_cast<double>(context.cameraPosition[2]);

    const double fov = static_cast<double>(context.fovY);
    const double viewportHeight = std::max(static_cast<double>(context.viewportHeight), 1.0);

    // Distance from the camera to the plane (perpendicular).
    double distPlane = std::abs(planeNormal[0] * (eyeX - planeOrigin[0]) +
                                planeNormal[1] * (eyeY - planeOrigin[1]) +
                                planeNormal[2] * (eyeZ - planeOrigin[2]));
    if (distPlane < 1e-6)
        distPlane = 1e-6;

    // Visible world height at the plane -> adaptive step (target ~16 px per
    // minor line; niceStep rounds up to 1/2/5 x 10^n, so we aim at half).
    const double worldHeight = 2.0 * distPlane * std::tan(fov * 0.5);
    double step = niceStep(worldHeight * 8.0 / viewportHeight);

    // Guarantee a minimum on-screen density of ~3 px.
    while (step / std::max(worldHeight, 1e-9) * viewportHeight < 3.0)
        step *= 2.0;

    const double majorStep = step * 5.0;
    const float minorStepF = static_cast<float>(step);
    const float majorStepF = static_cast<float>(majorStep);

    // Camera-relative anchor, snapped to the major step in double precision.
    double anchor[3] = {0.0, 0.0, 0.0};
    if (m_plane == GridPlane::XY)
    {
        anchor[0] = std::floor(eyeX / majorStep) * majorStep;
        anchor[1] = 0.0;
        anchor[2] = std::floor(eyeZ / majorStep) * majorStep;
    }
    else if (m_plane == GridPlane::XZ)
    {
        anchor[0] = std::floor(eyeX / majorStep) * majorStep;
        anchor[1] = std::floor(eyeY / majorStep) * majorStep;
        anchor[2] = 0.0;
    }
    else
    {
        anchor[0] = 0.0;
        anchor[1] = std::floor(eyeY / majorStep) * majorStep;
        anchor[2] = std::floor(eyeZ / majorStep) * majorStep;
    }

    // Fade must cover the whole visible region of the plane: rays near the
    // horizon travel far along the plane, so the fade radius is scaled by the
    // perpendicular camera-to-plane distance (farPlane stays a lower bound).
    const float fade = m_fadeDistanceOverride > 0.0f
        ? m_fadeDistanceOverride
        : static_cast<float>(std::max(
              static_cast<double>(context.farPlane) * 0.85,
              distPlane * 5.0));

    // Invert projection for view-space ray reconstruction.
    // Matrix4::inverse() (Gauss-Jordan with partial pivoting) is the canonical
    // implementation used by RayPicker; the hand-rolled 4x4 cofactor formula
    // previously used here produced a wrong inverse (P * inv != I).
    Matrix4Raw invProjection = IdentityMatrix4();
    {
        mir::Matrix4 projection;
        const float* m = context.projectionMatrix.data();
        for (int row = 0; row < 4; ++row)
            for (int column = 0; column < 4; ++column)
                projection(row, column) = static_cast<mir::Scalar>(m[row + column * 4]);
        const mir::Matrix4 inverse = projection.inverse();
        for (int row = 0; row < 4; ++row)
            for (int column = 0; column < 4; ++column)
                invProjection[row + column * 4] = static_cast<float>(inverse(row, column));
    }

    // Plane in view space: n_v . p = const_v
    // n_v = R * n_w ; const_v = n_w . (origin - eye)
    const float* v = context.viewMatrix.data();
    const float nv[3] = {
        v[0] * planeNormal[0] + v[4] * planeNormal[1] + v[8] * planeNormal[2],
        v[1] * planeNormal[0] + v[5] * planeNormal[1] + v[9] * planeNormal[2],
        v[2] * planeNormal[0] + v[6] * planeNormal[1] + v[10] * planeNormal[2]};
    const double planeConst = planeNormal[0] * (planeOrigin[0] - eyeX) +
                              planeNormal[1] * (planeOrigin[1] - eyeY) +
                              planeNormal[2] * (planeOrigin[2] - eyeZ);

    // World-from-view rotation (R^T) as a mat3.
    // v is column-major: v[c*4+r] = R[r][c]. GLSL mat3 is also column-major,
    // so to make the shader compute R^T * pView we must upload data[j*3+i] =
    // R[j][i] = v[i*4+j]; a naive row-wise copy here transposes the rotation
    // and mirrors the grid frame against the scene.
    float worldFromView[9] = {
        v[0], v[4], v[8],
        v[1], v[5], v[9],
        v[2], v[6], v[10]};

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    // The full-screen quad lives at NDC z = 1.0 (depth = 1.0), so the depth
    // test (GL_LESS) would reject every fragment against the cleared depth.
    // The grid is a background layer: draw it without the depth test.
    glDisable(GL_DEPTH_TEST);

    if (m_showGrid && m_quadVAO)
    {
        m_gridShader->bind();
        m_gridShader->setMatrix("uInvProjection", invProjection);
        m_gridShader->setVec2("uScreenSize",
                              static_cast<float>(context.viewportWidth),
                              static_cast<float>(context.viewportHeight));
        m_gridShader->setVec3("uCamPos",
                              context.cameraPosition[0],
                              context.cameraPosition[1],
                              context.cameraPosition[2]);
        m_gridShader->setVec3("uAnchor",
                              static_cast<float>(anchor[0]),
                              static_cast<float>(anchor[1]),
                              static_cast<float>(anchor[2]));
        m_gridShader->setVec3("uPlaneNormalView", nv[0], nv[1], nv[2]);
        m_gridShader->setVec3("uPlaneNormalWorld", planeNormal[0], planeNormal[1], planeNormal[2]);
        m_gridShader->setFloat("uPlaneConstView", static_cast<float>(planeConst));
        m_gridShader->setMatrix3("uWorldFromView", worldFromView);
        m_gridShader->setFloat("uMinorStep", minorStepF);
        m_gridShader->setFloat("uMajorStep", majorStepF);
        m_gridShader->setFloat("uFadeDistance", fade);
        m_gridShader->setFloat("uCameraHeight", static_cast<float>(distPlane));
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
        const float axisLength = fade * 0.75f;
        const float nx = (m_plane == GridPlane::YZ) ? 1.0f : 0.0f;
        const float ny = (m_plane == GridPlane::XZ) ? 1.0f : 0.0f;
        const float nz = (m_plane == GridPlane::XY) ? 1.0f : 0.0f;

        // Camera-relative view matrix (rotation only, no translation).
        Matrix4Raw viewRel = context.viewMatrix;
        viewRel[3] = 0.0f; viewRel[7] = 0.0f; viewRel[11] = 0.0f;
        Matrix4Raw vpRel = IdentityMatrix4();
        const float* p = context.projectionMatrix.data();
        for (int row = 0; row < 4; ++row)
            for (int col = 0; col < 4; ++col)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                    sum += p[row * 4 + k] * viewRel[k * 4 + col];
                vpRel[row * 4 + col] = sum;
            }

        m_axisShader->bind();
        m_axisShader->setMatrix("uVP", vpRel);
        m_axisShader->setVec3("uAxisDirection", nx, ny, nz);
        m_axisShader->setFloat("uAxisLength", axisLength);
        m_axisShader->setVec3("uColor", verticalColor[0], verticalColor[1], verticalColor[2]);
        m_axisShader->setFloat("uFadeDistance", fade);
        m_axisVAO->bind();
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, 2);
        m_axisVAO->unbind();
        m_axisShader->unbind();
        glLineWidth(1.0f);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

} // namespace MirEngine::Rendering