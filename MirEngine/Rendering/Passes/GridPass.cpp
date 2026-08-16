// MirEngine/Rendering/Passes/GridPass.cpp
// =================================================================================
// Engineering grid overlay.
//
// The grid is rebuilt on the CPU every frame as line quads. All geometry lives
// in camera-relative coordinates (world - camera position, computed in double
// precision) and is transformed by two separate uniforms:
//     gl_Position = u_projection * u_view * vec4(aPos, 1.0)
// where u_view is the rotation-only camera view matrix. This matches the
// GeometryPass contract, so the grid and the objects share the same math and
// cannot diverge.
// =================================================================================

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

// Full-screen background quad in NDC (drawn without a matrix).
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
    float color[3]{0.45f, 0.50f, 0.58f};
    float alpha0{0.75f};
    float alpha1{0.75f};
    float widthPx{2.5f};
};

} // namespace

// Line vertex shader. u_view is the rotation-only view matrix, u_projection is
// the projection matrix. Vertices are camera-relative.
static const char* kLineVS = R"(
#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aParams; // x = alpha, y = side profile (-1..1)

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_color;
out float v_alpha;
out float v_side;

void main() {
    v_color = aColor;
    v_alpha = aParams.x;
    v_side = aParams.y;
    gl_Position = u_projection * u_view * vec4(aPos, 1.0);
}
)";

// Soft alpha edge across the line width.
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
    vertices.reserve(4);
    for (int i = 0; i < 4; ++i)
    {
        Vertex vertex;
        vertex.position = {kBgQuadVertices[i * 3],
                           kBgQuadVertices[i * 3 + 1],
                           kBgQuadVertices[i * 3 + 2]};
        vertex.normal = {0.0f, 0.0f, 1.0f};
        vertex.uv = {0.0f, 0.0f};
        vertices.push_back(vertex);
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
                          double step) noexcept
{
    anchor[0] = anchor[1] = anchor[2] = 0.0;
    switch (plane)
    {
    case GridPlane::XY:
        normal[0] = 0.0f; normal[1] = 0.0f; normal[2] = 1.0f;
        uDir[0] = 1.0f; uDir[1] = 0.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 1.0f; vDir[2] = 0.0f;
        anchor[0] = std::floor(eye[0] / step) * step;
        anchor[1] = std::floor(eye[1] / step) * step;
        break;
    case GridPlane::XZ:
        normal[0] = 0.0f; normal[1] = 1.0f; normal[2] = 0.0f;
        uDir[0] = 1.0f; uDir[1] = 0.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 0.0f; vDir[2] = 1.0f;
        anchor[0] = std::floor(eye[0] / step) * step;
        anchor[2] = std::floor(eye[2] / step) * step;
        break;
    case GridPlane::YZ:
    default:
        normal[0] = 1.0f; normal[1] = 0.0f; normal[2] = 0.0f;
        uDir[0] = 0.0f; uDir[1] = 1.0f; uDir[2] = 0.0f;
        vDir[0] = 0.0f; vDir[1] = 0.0f; vDir[2] = 1.0f;
        anchor[1] = std::floor(eye[1] / step) * step;
        anchor[2] = std::floor(eye[2] / step) * step;
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
        // Rotation-only view matrix (camera translation is applied on the CPU
        // by making all vertex positions camera-relative).
        Matrix4Raw viewRelative = context.viewMatrix;
        viewRelative[12] = 0.0f;
        viewRelative[13] = 0.0f;
        viewRelative[14] = 0.0f;

        m_lineShader->bind();
        m_lineShader->setMatrix("u_view", viewRelative);
        m_lineShader->setMatrix("u_projection", context.projectionMatrix);
        m_gridVAO->bind();
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(m_gridIBO->getIndexCount()),
                       GL_UNSIGNED_INT,
                       nullptr);
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

    // The plane basis is resolved FIRST: everything below depends on it.
    float normal[3] = {0.0f, 0.0f, 1.0f};
    float uDir[3] = {1.0f, 0.0f, 0.0f};
    float vDir[3] = {0.0f, 1.0f, 0.0f};
    double anchor[3] = {0.0, 0.0, 0.0};
    planeBasis(m_plane, normal, uDir, vDir, anchor, eye, 1.0);

    const double fade = m_fadeDistanceOverride > 0.0
        ? static_cast<double>(m_fadeDistanceOverride)
        : static_cast<double>(context.farPlane) * 0.85;

    // Distance to the plane (the plane passes through the world origin).
    const double distPlane = std::abs(normal[0] * eye[0] +
                                      normal[1] * eye[1] +
                                      normal[2] * eye[2]);
    if (distPlane < 1e-9)
    {
        // Camera lies exactly on the grid plane: no well-defined view extent.
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

    // Frustum half-extents projected onto the grid plane (constant during
    // orbit, so the grid does not collapse at glancing angles).
    const double aV = fov * 0.5;
    const double cosV = std::cos(aV);
    const double sinV = std::sin(aV);
    const double denomHiV = std::max(std::abs(nF * cosV + nU * sinV), 1e-6);
    const double denomLoV = std::max(std::abs(nF * cosV - nU * sinV), 1e-6);
    const double tHiV = distPlane / denomHiV;
    const double tLoV = distPlane / denomLoV;
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

    // Adaptive step: ~18 px per cell at the viewed distance (1/2/5 x 10^n),
    // capped by the maximum line count.
    double step = niceStep(2.0 * viewDist * std::tan(aV) * 18.0 / viewportHeight);
    while ((2.0 * std::max(halfH, halfW) / step) > kMaxLinesPerAxis)
        step *= 2.0;

    // The anchor snaps to the current step, so the grid shifts by a single
    // line while panning instead of jumping by five lines at a time.
    planeBasis(m_plane, normal, uDir, vDir, anchor, eye, step);

    std::vector<GridSegment> segments;
    segments.reserve(600);

    if (m_showGrid)
    {
        const int countU = std::min(
            static_cast<int>(std::floor(halfW * kExtendFactor / step)),
            static_cast<int>(kMaxLinesPerAxis));
        const int countV = std::min(
            static_cast<int>(std::floor(halfH * kExtendFactor / step)),
            static_cast<int>(kMaxLinesPerAxis));

        const double extendU = halfW * kExtendFactor;
        const double extendV = halfH * kExtendFactor;

        // Lines of constant u (running along v).
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
                segment.color[0] = 0.65f; segment.color[1] = 0.70f; segment.color[2] = 0.78f;
                segment.alpha0 = 0.95f; segment.alpha1 = 0.95f;
                segment.widthPx = 3.0f;
            }
            else
            {
                segment.color[0] = 0.45f; segment.color[1] = 0.50f; segment.color[2] = 0.58f;
                segment.alpha0 = 0.75f; segment.alpha1 = 0.75f;
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
                segment.color[0] = 0.65f; segment.color[1] = 0.70f; segment.color[2] = 0.78f;
                segment.alpha0 = 0.95f; segment.alpha1 = 0.95f;
                segment.widthPx = 3.0f;
            }
            else
            {
                segment.color[0] = 0.45f; segment.color[1] = 0.50f; segment.color[2] = 0.58f;
                segment.alpha0 = 0.75f; segment.alpha1 = 0.75f;
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
            segment.alpha1 = 0.55f;
            segment.widthPx = 2.5f;
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

    const double fadeStart = fade * 0.70;
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

        // Perpendicular distance from the camera to the infinite line: the
        // correct quantity for both fade and screen-space width.
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
            // Grid lines: fade by the line distance, so lines at the horizon
            // vanish smoothly while lines under the camera never disappear.
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

        // Expansion direction: perpendicular to both the segment direction and
        // the view ray, so the quad faces the camera.
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
