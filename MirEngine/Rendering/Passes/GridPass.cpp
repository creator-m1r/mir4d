#include "GridPass.h"
#include "../OpenGL/OpenGLShader.h"
#include "../Resources/VertexArray.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"
#include "../Resources/Vertex.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"

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

// Full-screen background quad in NDC.
constexpr float kBgQuadVertices[] = {
    -1.0f, -1.0f, 1.0f,
     1.0f, -1.0f, 1.0f,
    -1.0f,  1.0f, 1.0f,
     1.0f,  1.0f, 1.0f,
};

constexpr unsigned int kBgQuadIndices[] = {0, 1, 2, 1, 3, 2};

// One grid segment (world space, double precision).
struct GridSegment
{
    double p0[3]{0.0, 0.0, 0.0};
    double p1[3]{0.0, 0.0, 0.0};
    float color[3]{0.22f, 0.25f, 0.30f};
    float alpha0{0.5f};
    float alpha1{0.5f};
    float widthPx{1.0f};
};

} // namespace

// Line vertex shader. Vertices arrive camera-relative (world - camera
// position, computed in double on the CPU), u_viewProj is the full projection
// times the rotation-only view matrix, so all GPU numbers stay small.
static const char* kLineVS = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aParams; // x = alpha, y = side profile (-1..1)

uniform mat4 u_viewProj;

out vec3 v_color;
out float v_alpha;
out float v_side;

void main() {
    v_color = aColor;
    v_alpha = aParams.x;
    v_side = aParams.y;
    gl_Position = u_viewProj * vec4(aPos, 1.0);
}
)";

// Soft alpha edge across the line width: the quad is ~1-2 px wide, the side
// profile grades the alpha, so lines stay smooth without MSAA.
static const char* kLineFS = R"(
#version 410 core
in vec3 v_color;
in float v_alpha;
in float v_side;
out vec4 FragColor;

void main() {
    float edge = 1.0 - smoothstep(0.45, 1.0, abs(v_side));
    float alpha = v_alpha * edge;
    if (alpha <= 0.004) discard;
    FragColor = vec4(v_color, alpha);
}
)";

static const char* kBgVS = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
out vec2 vUv;
void main() {
    vUv = aPos.xy * 0.5 + 0.5;
    gl_Position = vec4(aPos, 1.0);
}
)";

static const char* kBgFS = R"(
#version 410 core
in vec2 vUv;
out vec4 FragColor;
uniform vec3 uTopColor;
uniform vec3 uBottomColor;
void main() {
    float t = clamp(vUv.y, 0.0, 1.0);
    float k = pow(t, 0.85);
    FragColor = vec4(mix(uBottomColor, uTopColor, k), 1.0);
}
)";

GridPass::GridPass() = default;
GridPass::~GridPass() = default;

bool GridPass::initialize(RenderDevice& device)
{
    if (m_initialized)
        return true;

    if (!createShaders())
    {
        std::cerr << "[GridPass] Failed to create shaders\n";
        return false;
    }
    buildBackground(device);
    m_initialized = true;
    return true;
}

bool GridPass::createShaders()
{
    m_lineShader = std::make_unique<OpenGLShader>();
    if (!m_lineShader->compile(kLineVS, kLineFS))
    {
        m_lineShader.reset();
        return false;
    }

    m_bgShader = std::make_unique<OpenGLShader>();
    if (!m_bgShader->compile(kBgVS, kBgFS))
    {
        m_bgShader.reset();
        return false;
    }
    return true;
}

void GridPass::buildBackground(RenderDevice& device)
{
    std::vector<Vertex> vertices;
    for (int i = 0; i < 4; ++i)
    {
        vertices.push_back({{kBgQuadVertices[i * 3],
                             kBgQuadVertices[i * 3 + 1],
                             kBgQuadVertices[i * 3 + 2]},
                            {0.0f, 1.0f, 0.0f},
                            {0.0f, 0.0f}});
    }
    std::vector<std::uint32_t> indices = {
        kBgQuadIndices[0], kBgQuadIndices[1], kBgQuadIndices[2],
        kBgQuadIndices[3], kBgQuadIndices[4], kBgQuadIndices[5]};

    m_bgVBO = device.createVertexBuffer();
    m_bgVBO->uploadVertices(vertices, BufferUsage::Static);
    m_bgIBO = device.createIndexBuffer();
    m_bgIBO->uploadIndices(indices, BufferUsage::Static);
    m_bgVAO = device.createVertexArray();
    m_bgVAO->setVertexBuffer(m_bgVBO);
    m_bgVAO->setIndexBuffer(m_bgIBO);
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
                          float uDir[3],
                          float vDir[3],
                          double anchor[3],
                          const double eye[3],
                          double majorStep) noexcept
{
    anchor[0] = anchor[1] = anchor[2] = 0.0;
    switch (plane)
    {
    case GridPlane::XY:
        normal[0] = 0.0f; normal[1] = 0.0f; normal[2] = 1.0f;
        uDir[0] = 1.0f; uDir[1] = 0.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 1.0f; vDir[2] = 0.0f;
        anchor[0] = std::floor(eye[0] / majorStep) * majorStep;
        anchor[1] = std::floor(eye[1] / majorStep) * majorStep;
        break;
    case GridPlane::XZ:
        normal[0] = 0.0f; normal[1] = 1.0f; normal[2] = 0.0f;
        uDir[0] = 1.0f; uDir[1] = 0.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 0.0f; vDir[2] = 1.0f;
        anchor[0] = std::floor(eye[0] / majorStep) * majorStep;
        anchor[2] = std::floor(eye[2] / majorStep) * majorStep;
        break;
    case GridPlane::YZ:
    default:
        normal[0] = 1.0f; normal[1] = 0.0f; normal[2] = 0.0f;
        uDir[0] = 0.0f; uDir[1] = 1.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 0.0f; vDir[2] = 1.0f;
        anchor[1] = std::floor(eye[1] / majorStep) * majorStep;
        anchor[2] = std::floor(eye[2] / majorStep) * majorStep;
        break;
    }
}

void GridPass::execute(RenderContext& context,
                       mir::Scene&,
                       RenderDevice& device)
{
    if (!m_initialized || !m_lineShader)
        return;

    // ------------------------------------------------------------------
    // 1. Opaque studio gradient background.
    // ------------------------------------------------------------------
    if (m_bgShader && m_bgVAO)
    {
        device.setDepthTest(false);
        device.setBlend(false);
        m_bgShader->bind();
        m_bgShader->setVec3("uTopColor", 0.16f, 0.19f, 0.24f);
        m_bgShader->setVec3("uBottomColor", 0.03f, 0.04f, 0.06f);
        m_bgVAO->bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        m_bgVAO->unbind();
        m_bgShader->unbind();
    }

    // ------------------------------------------------------------------
    // 2. Grid and axis lines (CPU-generated quads).
    // ------------------------------------------------------------------
    device.setDepthTest(false);
    device.setBlend(true);
    device.setCullFace(false); // line quads are double-sided

    rebuildLines(context, device);

    if (m_gridVAO && m_gridVBO && m_gridIBO && m_gridVBO->getVertexCount() > 0)
    {
        Matrix4Raw viewRelative = context.viewMatrix;
        viewRelative[12] = 0.0f;
        viewRelative[13] = 0.0f;
        viewRelative[14] = 0.0f;

        Matrix4Raw viewProj{};
        const float* proj = context.projectionMatrix.data();
        const float* view = viewRelative.data();
        for (int row = 0; row < 4; ++row)
            for (int column = 0; column < 4; ++column)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                    sum += proj[row * 4 + k] * view[k * 4 + column];
                viewProj[row * 4 + column] = sum;
            }

        m_lineShader->bind();
        m_lineShader->setMatrix("u_viewProj", viewProj);
        m_gridVAO->bind();
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(m_gridIBO->getIndexCount()),
                       GL_UNSIGNED_INT,
                       nullptr);
        static int s_diag2 = 0;
        if (++s_diag2 % 60 == 0)
        {
            GLenum err = glGetError();
            std::cerr << "[GridPass] draw idx=" << m_gridIBO->getIndexCount()
                      << " verts=" << m_gridVBO->getVertexCount()
                      << " glError=0x" << std::hex << err << std::dec
                      << " vp0=" << viewProj[0] << " vp5=" << viewProj[5] << " vp10=" << viewProj[10] << " vp15=" << viewProj[15] << "\n";
        }
        m_gridVAO->unbind();
        m_lineShader->unbind();
    }

    // Restore defaults for the geometry pass.
    device.setDepthTest(true);
    device.setBlend(false);
    device.setCullFace(true);
}

void GridPass::rebuildLines(RenderContext& context, RenderDevice& device)
{
    if (!m_showGrid && !m_showAxes)
    {
        m_gridVBO.reset();
        m_gridIBO.reset();
        m_gridVAO.reset();
        return;
    }

    const double eye[3] = {
        static_cast<double>(context.cameraPosition[0]),
        static_cast<double>(context.cameraPosition[1]),
        static_cast<double>(context.cameraPosition[2])};

    const double fov = static_cast<double>(context.fovY);
    const double viewportHeight = std::max(static_cast<double>(context.viewportHeight), 1.0);
    const double aspect = viewportHeight > 0
        ? static_cast<double>(context.viewportWidth) / viewportHeight
        : 1.0;

    float normal[3] = {0.0f, 0.0f, 1.0f};
    float uDir[3] = {1.0f, 0.0f, 0.0f};
    float vDir[3] = {0.0f, 1.0f, 0.0f};

    const double fade = m_fadeDistanceOverride > 0.0
        ? static_cast<double>(m_fadeDistanceOverride)
        : static_cast<double>(context.farPlane) * 0.85;

    // Distance to the plane (the plane passes through the world origin).
    const double distPlane = std::abs(normal[0] * eye[0] +
                                      normal[1] * eye[1] +
                                      normal[2] * eye[2]);
    if (distPlane < 1e-9)
    {
        // Camera lies exactly on the grid plane: the frustum does not
        // intersect the plane at a well-defined extent. Skip this frame.
        m_gridVBO.reset();
        m_gridIBO.reset();
        m_gridVAO.reset();
        return;
    }

    // Camera basis in world space from the rotation-only view matrix
    // (column-major: right = rows 0-2, up = rows 4-6, forward = -rows 8-10).
    const double forward[3] = {
        -static_cast<double>(context.viewMatrix[8]),
        -static_cast<double>(context.viewMatrix[9]),
        -static_cast<double>(context.viewMatrix[10])};
    const double up[3] = {
        static_cast<double>(context.viewMatrix[4]),
        static_cast<double>(context.viewMatrix[5]),
        static_cast<double>(context.viewMatrix[6])};
    const double right[3] = {
        static_cast<double>(context.viewMatrix[0]),
        static_cast<double>(context.viewMatrix[1]),
        static_cast<double>(context.viewMatrix[2])};

    const auto dot3 = [](const double a[3], const double b[3]) -> double
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    };
    const double nrm[3] = {
        static_cast<double>(normal[0]),
        static_cast<double>(normal[1]),
        static_cast<double>(normal[2])};
    const double nF = dot3(forward, nrm);
    const double nU = dot3(up, nrm);
    const double nR = dot3(right, nrm);
    const double viewDist = distPlane / std::max(std::abs(nF), 1e-4);

    // Frustum half-extents projected onto the grid plane. The visible region
    // grows as the camera looks along the plane; the naive
    // distPlane * tan(fov / 2) estimate would shrink the grid while orbiting
    // (the "collapsing grid" artifact).
    const double aV = fov * 0.5;
    const double cosV = std::cos(aV);
    const double sinV = std::sin(aV);
    const double denomHiV = std::max(std::abs(nF * cosV + nU * sinV), 1e-6);
    const double denomLoV = std::max(std::abs(nF * cosV - nU * sinV), 1e-6);
    const double tHiV = distPlane / denomHiV;
    const double tLoV = distPlane / denomLoV;
    // Cap the frustum extents at the fade distance: beyond it every line is
    // invisible anyway, and an unbounded extent (a frustum edge parallel to
    // the plane) would overflow the line counters below.
    const double halfH = std::min(
        0.5 * std::sqrt((tHiV - tLoV) * (tHiV - tLoV) * cosV * cosV +
                        (tHiV + tLoV) * (tHiV + tLoV) * sinV * sinV),
        fade);

    const double aH = std::atan(std::tan(aV) * aspect);
    const double cosH = std::cos(aH);
    const double sinH = std::sin(aH);
    const double denomHiH = std::max(std::abs(nF * cosH + nR * sinH), 1e-6);
    const double denomLoH = std::max(std::abs(nF * cosH - nR * sinH), 1e-6);
    const double tHiH = distPlane / denomHiH;
    const double tLoH = distPlane / denomLoH;
    const double halfW = std::min(
        0.5 * std::sqrt((tHiH - tLoH) * (tHiH - tLoH) * cosH * cosH +
                        (tHiH + tLoH) * (tHiH + tLoH) * sinH * sinH),
        fade);

    // Adaptive step: ~18 px per cell at the viewed distance, which stays
    // constant while orbiting, so the grid scale no longer jumps with the
    // view angle; 1/2/5 x 10^n; capped line count.
    double step = niceStep(2.0 * viewDist * std::tan(aV) * 18.0 / viewportHeight);
    while ((2.0 * std::max(halfH, halfW) / step) > kMaxLinesPerAxis)
        step *= 2.0;
    const double majorStep = step * 5.0;

    // The anchor snaps to the current step, so the grid shifts by a single
    // line while panning instead of jumping by five lines at a time.
    double anchor[3] = {0.0, 0.0, 0.0};
    planeBasis(m_plane, normal, uDir, vDir, anchor, eye, step);

    std::vector<GridSegment> segments;
    segments.reserve(600);

    int countU = 0;
    int countV = 0;

    if (m_showGrid)
    {
        // Coordinates along the plane axes, anchored at the camera.
        countU = std::min(
            static_cast<int>(std::floor(halfW * kExtendFactor / step)),
            static_cast<int>(kMaxLinesPerAxis));
        countV = std::min(
            static_cast<int>(std::floor(halfH * kExtendFactor / step)),
            static_cast<int>(kMaxLinesPerAxis));

        const double extendU = halfW * kExtendFactor;
        const double extendV = halfH * kExtendFactor;

        // Lines of constant u (running along v). Coordinates are measured
        // from the anchor (the grid cell under the camera), so the grid
        // follows the camera and always covers the viewport.
        for (int k = -countU; k <= countU; ++k)
        {
            const double u = static_cast<double>(k) * step;
            const bool major = (std::abs(k) % 5 == 0);

            GridSegment segment;
            for (int i = 0; i < 3; ++i)
            {
                segment.p0[i] = anchor[i] + uDir[i] * u - vDir[i] * extendV;
                segment.p1[i] = anchor[i] + uDir[i] * u + vDir[i] * extendV;
            }
            if (major)
            {
                segment.color[0] = 0.38f; segment.color[1] = 0.42f; segment.color[2] = 0.50f;
                segment.alpha0 = 0.9f; segment.alpha1 = 0.9f;
                segment.widthPx = 3.0f;
            }
            else
            {
                segment.color[0] = 0.22f; segment.color[1] = 0.25f; segment.color[2] = 0.30f;
                segment.alpha0 = 0.55f; segment.alpha1 = 0.55f;
                segment.widthPx = 2.5f;
            }
            segments.push_back(segment);
        }

        // Lines of constant v (running along u).
        for (int k = -countV; k <= countV; ++k)
        {
            const double v = static_cast<double>(k) * step;
            const bool major = (std::abs(k) % 5 == 0);

            GridSegment segment;
            for (int i = 0; i < 3; ++i)
            {
                segment.p0[i] = anchor[i] + vDir[i] * v - uDir[i] * extendU;
                segment.p1[i] = anchor[i] + vDir[i] * v + uDir[i] * extendU;
            }
            if (major)
            {
                segment.color[0] = 0.38f; segment.color[1] = 0.42f; segment.color[2] = 0.50f;
                segment.alpha0 = 0.9f; segment.alpha1 = 0.9f;
                segment.widthPx = 3.0f;
            }
            else
            {
                segment.color[0] = 0.22f; segment.color[1] = 0.25f; segment.color[2] = 0.30f;
                segment.alpha0 = 0.55f; segment.alpha1 = 0.55f;
                segment.widthPx = 2.5f;
            }
            segments.push_back(segment);
        }
    }

    // Workspace axes from the world origin (X red, Y green, Z blue).
    if (m_showAxes)
    {
        const double axisLength = fade * 0.75;
        const float axisColors[3][3] = {
            {0.92f, 0.30f, 0.30f},
            {0.30f, 0.85f, 0.35f},
            {0.28f, 0.48f, 0.95f}};
        for (int axis = 0; axis < 3; ++axis)
        {
            GridSegment segment;
            for (int i = 0; i < 3; ++i)
            {
                segment.p0[i] = 0.0;
                segment.p1[i] = (axis == i) ? axisLength : 0.0;
                segment.color[i] = axisColors[axis][i];
            }
            segment.alpha0 = 0.95f;
            segment.alpha1 = 0.0f;
            segment.widthPx = 2.0f;
            segments.push_back(segment);
        }
    }

    if (segments.empty())
    {
        m_gridVBO.reset();
        m_gridIBO.reset();
        m_gridVAO.reset();
        return;
    }

    // Per-segment: distance fade, screen-space width and quad expansion.
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(segments.size() * 4);
    indices.reserve(segments.size() * 6);

    const double fadeStart = fade * 0.55;
    const double range = std::max(fade - fadeStart, 1e-9);
    const double tanHalfFov = std::tan(fov * 0.5);

    for (const GridSegment& segment : segments)
    {
        double segDir[3] = {
            segment.p1[0] - segment.p0[0],
            segment.p1[1] - segment.p0[1],
            segment.p1[2] - segment.p0[2]};
        double segLen = std::sqrt(segDir[0] * segDir[0] + segDir[1] * segDir[1] + segDir[2] * segDir[2]);
        if (segLen < 1e-9)
            continue;
        segDir[0] /= segLen; segDir[1] /= segLen; segDir[2] /= segLen;

        // Perpendicular distance from the camera to the infinite line. This
        // is the correct quantity for both fade and screen-space width: a
        // line passing close to the camera stays 1 px, no matter how far its
        // endpoints are (the "thick line" artifact).
        const double d0 = std::sqrt((segment.p0[0] - eye[0]) * (segment.p0[0] - eye[0]) +
                                    (segment.p0[1] - eye[1]) * (segment.p0[1] - eye[1]) +
                                    (segment.p0[2] - eye[2]) * (segment.p0[2] - eye[2]));
        const double proj = (segment.p0[0] - eye[0]) * segDir[0] +
                            (segment.p0[1] - eye[1]) * segDir[1] +
                            (segment.p0[2] - eye[2]) * segDir[2];
        const double dPerpSq = d0 * d0 - proj * proj;
        const double dPerp = (dPerpSq > 0.0) ? std::sqrt(dPerpSq) : 0.0;

        float a0;
        float a1;
        if (segment.alpha0 == segment.alpha1)
        {
            // Grid lines: fade by the line distance, so lines at the
            // horizon vanish smoothly and lines under the camera never
            // disappear because their endpoints are far away.
            const float aLine = segment.alpha0 * static_cast<float>(
                1.0 - std::clamp((dPerp - fadeStart) / range, 0.0, 1.0));
            if (aLine <= 0.004f)
                continue;
            a0 = aLine;
            a1 = aLine;
        }
        else
        {
            // Workspace axes keep their per-vertex gradient.
            const double d1 = std::sqrt((segment.p1[0] - eye[0]) * (segment.p1[0] - eye[0]) +
                                        (segment.p1[1] - eye[1]) * (segment.p1[1] - eye[1]) +
                                        (segment.p1[2] - eye[2]) * (segment.p1[2] - eye[2]));
            a0 = segment.alpha0 * static_cast<float>(1.0 - std::clamp((d0 - fadeStart) / range, 0.0, 1.0));
            a1 = segment.alpha1 * static_cast<float>(1.0 - std::clamp((d1 - fadeStart) / range, 0.0, 1.0));
            if (a0 <= 0.004f && a1 <= 0.004f)
                continue;
        }

        // Screen-space width: ~widthPx pixels at the perpendicular distance.
        const double pxToWorld = 2.0 * dPerp * tanHalfFov / viewportHeight;
        const double halfWidth = pxToWorld * static_cast<double>(segment.widthPx) * 0.5;

        const double mid[3] = {
            (segment.p0[0] + segment.p1[0]) * 0.5,
            (segment.p0[1] + segment.p1[1]) * 0.5,
            (segment.p0[2] + segment.p1[2]) * 0.5};
        double viewDir[3] = {-mid[0], -mid[1], -mid[2]};
        double viewLen = std::sqrt(viewDir[0] * viewDir[0] + viewDir[1] * viewDir[1] + viewDir[2] * viewDir[2]);
        if (viewLen < 1e-9)
            continue;
        viewDir[0] /= viewLen; viewDir[1] /= viewLen; viewDir[2] /= viewLen;

        double perp[3] = {
            segDir[1] * viewDir[2] - segDir[2] * viewDir[1],
            segDir[2] * viewDir[0] - segDir[0] * viewDir[2],
            segDir[0] * viewDir[1] - segDir[1] * viewDir[0]};
        double perpLen = std::sqrt(perp[0] * perp[0] + perp[1] * perp[1] + perp[2] * perp[2]);
        if (perpLen < 1e-6)
        {
            // Line nearly parallel to the view ray: pick any perpendicular.
            perp[0] = (std::abs(segDir[1]) < 0.5) ? 1.0 : 0.0;
            perp[1] = (std::abs(segDir[0]) < 0.5) ? 1.0 : 0.0;
            perp[2] = (std::abs(segDir[0]) < 0.5 && std::abs(segDir[1]) < 0.5) ? 1.0 : 0.0;
            perpLen = std::sqrt(perp[0] * perp[0] + perp[1] * perp[1] + perp[2] * perp[2]);
            if (perpLen < 1e-6)
                continue;
        }
        perp[0] /= perpLen; perp[1] /= perpLen; perp[2] /= perpLen;

        const auto pack = [&](Vertex& vertex, double px, double py, double pz,
                              float alpha, float side)
        {
            vertex.position = {
                static_cast<float>(px - eye[0]),
                static_cast<float>(py - eye[1]),
                static_cast<float>(pz - eye[2])};
            vertex.normal = {segment.color[0], segment.color[1], segment.color[2]};
            vertex.uv = {alpha, side};
        };

        Vertex quad[4];
        pack(quad[0], segment.p0[0] - perp[0] * halfWidth,
                      segment.p0[1] - perp[1] * halfWidth,
                      segment.p0[2] - perp[2] * halfWidth, a0, -1.0f);
        pack(quad[1], segment.p0[0] + perp[0] * halfWidth,
                      segment.p0[1] + perp[1] * halfWidth,
                      segment.p0[2] + perp[2] * halfWidth, a0, 1.0f);
        pack(quad[2], segment.p1[0] - perp[0] * halfWidth,
                      segment.p1[1] - perp[1] * halfWidth,
                      segment.p1[2] - perp[2] * halfWidth, a1, -1.0f);
        pack(quad[3], segment.p1[0] + perp[0] * halfWidth,
                      segment.p1[1] + perp[1] * halfWidth,
                      segment.p1[2] + perp[2] * halfWidth, a1, 1.0f);

        const std::uint32_t base = static_cast<std::uint32_t>(vertices.size());
        vertices.insert(vertices.end(), std::begin(quad), std::end(quad));
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }

    if (vertices.empty() || indices.empty())
    {
        m_gridVBO.reset();
        m_gridIBO.reset();
        m_gridVAO.reset();
        return;
    }

    static int s_diagFrames = 0;
    if (++s_diagFrames % 60 == 0)
    {
        std::cerr << "[GridPass] step=" << step
                  << " halfH=" << halfH << " halfW=" << halfW
                  << " countU=" << countU << " countV=" << countV
                  << " segs=" << segments.size()
                  << " verts=" << vertices.size()
                  << " anchor=" << anchor[0] << "," << anchor[1] << "," << anchor[2] << "\n";
    }

    if (!m_gridVBO)
    {
        m_gridVBO = device.createVertexBuffer();
        m_gridIBO = device.createIndexBuffer();
        m_gridVAO = device.createVertexArray();
        m_gridVAO->setVertexBuffer(m_gridVBO);
        m_gridVAO->setIndexBuffer(m_gridIBO);
    }
    m_gridVBO->uploadVertices(vertices, BufferUsage::Dynamic);
    m_gridIBO->uploadIndices(indices, BufferUsage::Dynamic);
}

} // namespace MirEngine::Rendering