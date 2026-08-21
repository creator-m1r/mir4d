
#include "HandSkeletonPass.h"

#include "../Core/RenderCommand.h"
#include "../Core/RenderContext.h"
#include "../Core/RenderDevice.h"
#include "../Resources/Vertex.h"
#include "../OpenGL/OpenGLShader.h"
#include "../Resources/VertexBuffer.h"
#include "../Resources/VertexArray.h"
#include "MirEngine/Geometry/Scene/Scene.hpp"

#if defined(__APPLE__)
#    include <OpenGL/gl3.h>
#else
#    include <glad/gl.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

namespace MirEngine::Rendering
{
namespace
{

constexpr char kVertSrc[] = R"GLSL(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;   // joint / bone colour (rgb)
layout(location = 2) in vec2 aData;    // x = alpha, y = point size
uniform mat4 u_viewProj;
out vec3 v_color;
out float v_alpha;
void main()
{
    v_color = aColor;
    v_alpha = aData.x;
    gl_Position = u_viewProj * vec4(aPos, 1.0);
    gl_PointSize = max(aData.y, 1.0);
}
)GLSL";

constexpr char kFragSrc[] = R"GLSL(
#version 410 core
in vec3 v_color;
in float v_alpha;
out vec4 FragColor;
void main()
{
    if (v_alpha <= 0.01) discard;
    FragColor = vec4(v_color, v_alpha);
}
)GLSL";

inline Vector3 lerp(const Vector3& a, const Vector3& b, float t) noexcept
{
    return Vector3(a.x + (b.x - a.x) * t,
                   a.y + (b.y - a.y) * t,
                   a.z + (b.z - a.z) * t);
}

}

std::vector<std::pair<int, int>> HandSkeletonPass::defaultBoneIndices()
{
    return {
        {0, 1},  {0, 5},  {0, 9},  {0, 13}, {0, 17},
        {1, 2},  {2, 3},  {3, 4},
        {5, 6},  {6, 7},  {7, 8},
        {9, 10}, {10, 11}, {11, 12},
        {13, 14}, {14, 15}, {15, 16},
        {17, 18}, {18, 19}, {19, 20},
        {5, 9},  {9, 13}, {13, 17}
    };
}

void HandSkeletonPass::setTopology(const std::vector<std::pair<int, int>>& bones)
{
    if (!bones.empty())
        m_boneIndices = bones;
}

void HandSkeletonPass::gestureAccent(int gestureCode, float out[3]) noexcept
{
    switch (gestureCode)
    {
        case 2:
        case 13:
            out[0] = 1.0f; out[1] = 0.85f; out[2] = 0.20f; break;
        case 3:
        case 14:
            out[0] = 0.30f; out[1] = 1.00f; out[2] = 0.40f; break;
        case 1:
            out[0] = 0.60f; out[1] = 0.80f; out[2] = 1.00f; break;
        default:
            out[0] = 1.0f; out[1] = 1.0f; out[2] = 1.0f; break;
    }
}

HandSkeletonPass::~HandSkeletonPass() = default;

bool HandSkeletonPass::initialize(RenderDevice& device)
{
    if (m_initialized)
        return true;

    m_shader = std::make_unique<OpenGLShader>();
    if (!m_shader->compile(kVertSrc, kFragSrc))
    {
        std::cerr << "[HandSkeletonPass] Failed to compile shader\n";
        return false;
    }

    m_pointVBO = device.createVertexBuffer();
    m_pointVAO = device.createVertexArray();
    m_pointVAO->setVertexBuffer(m_pointVBO);

    m_lineVBO = device.createVertexBuffer();
    m_lineVAO = device.createVertexArray();
    m_lineVAO->setVertexBuffer(m_lineVBO);

    if (!m_pointVBO || !m_pointVAO || !m_lineVBO || !m_lineVAO)
    {
        std::cerr << "[HandSkeletonPass] Failed to allocate buffers\n";
        return false;
    }

    m_initialized = true;
    return true;
}

void HandSkeletonPass::execute(RenderContext& context,
                               mir::Scene& ,
                               RenderDevice& device)
{
    if (!m_initialized)
        return;

    const HandSkeletonRenderData& sk = context.handSkeleton;
    if (sk.mode <= 0 || sk.handCount <= 0)
        return;

    const int handCount = std::min(sk.handCount, HandSkeletonRenderData::kMaxHands);

    std::vector<Vertex> points;
    std::vector<Vertex> lines;

    const bool drawBones = sk.mode >= 2;
    const bool drawRay = sk.mode >= 3;

    for (int h = 0; h < handCount; ++h)
    {
        const bool isRight = sk.handedness[h] == 1;
        const float* baseCol3 = isRight ? m_style.rightColor : m_style.leftColor;
        const Vector3 baseCol(baseCol3[0], baseCol3[1], baseCol3[2]);
        const float pinch = sk.pinch[h];
        const float* pos = sk.positions[h];
        const float* conf = sk.confidence[h];
        const float alpha = m_style.alpha;

        float accent[3]{};
        gestureAccent(sk.gesture[h], accent);
        const Vector3 accentCol(accent[0], accent[1], accent[2]);

        for (int j = 0; j < HandSkeletonRenderData::kMaxJoints; ++j)
        {
            const float c = conf[j];
            if (c < kMinConfidence)
                continue;
            const bool isTip = (j == 4 || j == 8 || j == 12 || j == 16 || j == 20);
            Vector3 col = baseCol;

            if (isTip)
                col = lerp(baseCol, accentCol, std::min(pinch, 1.0f) * 0.8f);
            const float size = (j == 0) ? m_style.wristSize
                                       : (isTip ? m_style.tipSize : m_style.jointSize);
            const Vector3 p(pos[j * 3], pos[j * 3 + 1], pos[j * 3 + 2]);
            points.push_back(Vertex(p, col, Vector2(c * alpha, size)));
        }

        if (!drawBones)
            continue;

        for (const auto& bone : m_boneIndices)
        {
            const float ca = conf[bone.first];
            const float cb = conf[bone.second];
            if (ca < kMinConfidence || cb < kMinConfidence)
                continue;
            const float a = std::min(ca, cb) * alpha;
            const Vector3 pa(pos[bone.first * 3], pos[bone.first * 3 + 1], pos[bone.first * 3 + 2]);
            const Vector3 pb(pos[bone.second * 3], pos[bone.second * 3 + 1], pos[bone.second * 3 + 2]);
            lines.push_back(Vertex(pa, baseCol, Vector2(a, 0.0f)));
            lines.push_back(Vertex(pb, baseCol, Vector2(a, 0.0f)));
        }

        if (pinch > 0.05f)
        {
            const float pa2 = conf[4];
            const float pb2 = conf[8];
            if (pa2 >= kMinConfidence && pb2 >= kMinConfidence)
            {
                const Vector3 a(pos[4 * 3], pos[4 * 3 + 1], pos[4 * 3 + 2]);
                const Vector3 b(pos[8 * 3], pos[8 * 3 + 1], pos[8 * 3 + 2]);
                const Vector3 mid = lerp(a, b, 0.5f);
                const Vector3 lineCol = lerp(baseCol, accentCol, 0.6f);
                const float lineAlpha = alpha * std::min(1.0f, pinch * 1.2f);
                lines.push_back(Vertex(a, lineCol, Vector2(pa2 * lineAlpha, 0.0f)));
                lines.push_back(Vertex(b, lineCol, Vector2(pb2 * lineAlpha, 0.0f)));
                lines.push_back(Vertex(mid, lineCol, Vector2(lineAlpha, 0.0f)));
                lines.push_back(Vertex(mid, lineCol, Vector2(lineAlpha, 0.0f)));
            }
        }

        if (drawRay)
        {

            const Vector3 wrist(pos[0], pos[1], pos[2]);
            const Vector3 tip(pos[8 * 3], pos[8 * 3 + 1], pos[8 * 3 + 2]);
            const Vector3 dir = Vector3(tip.x - wrist.x, tip.y - wrist.y, tip.z - wrist.z);
            const Vector3 end = Vector3(wrist.x + dir.x * 1.8f,
                                        wrist.y + dir.y * 1.8f,
                                        wrist.z + dir.z * 1.8f);
            const Vector3 rcol(1.0f, 1.0f, 0.4f);
            lines.push_back(Vertex(wrist, rcol, Vector2(0.9f * alpha, 0.0f)));
            lines.push_back(Vertex(end, rcol, Vector2(0.9f * alpha, 0.0f)));
        }
    }

    if (points.empty() && lines.empty())
        return;

    device.setCullFace(false);
    device.setBlend(true);
    if (m_style.depthTest)
    {
        device.setDepthTest(true);
        glDepthMask(GL_TRUE);
    }
    else
    {
        device.setDepthTest(false);
        glDepthMask(GL_FALSE);
    }

    m_shader->bind();
    m_shader->setMatrix("u_viewProj", context.viewProjectionMatrix);

    if (!points.empty())
    {
        m_pointVBO->uploadVertices(points, BufferUsage::Dynamic);
        m_pointVAO->bind();
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));
        m_pointVAO->unbind();
    }

    if (!lines.empty())
    {
        m_lineVBO->uploadVertices(lines, BufferUsage::Dynamic);
        m_lineVAO->bind();
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size()));
        m_lineVAO->unbind();
    }

    m_shader->unbind();

    glDepthMask(GL_TRUE);
    device.setDepthTest(true);
    device.setBlend(false);
    device.setCullFace(true);
}

}
