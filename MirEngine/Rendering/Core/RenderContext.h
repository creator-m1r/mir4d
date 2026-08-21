
#pragma once

#include <cstdint>
#include <vector>

#include "RenderCommand.h"

namespace MirEngine::Rendering {

struct PlaneRenderData
{
    std::uint32_t id{0};
    float origin[3]{0.0f, 0.0f, 0.0f};
    float normal[3]{0.0f, 0.0f, 1.0f};
    float xAxis[3]{1.0f, 0.0f, 0.0f};
    float yAxis[3]{0.0f, 1.0f, 0.0f};
    float color[3]{0.45f, 0.55f, 0.75f};
    float size{10.0f};
    bool active{false};
    bool selected{false};
    bool hovered{false};
};

struct SketchSegment2D
{
    float ax{0.0f};
    float ay{0.0f};
    float bx{0.0f};
    float by{0.0f};
    float color[3]{0.95f, 0.85f, 0.25f};
};

struct SketchRenderData
{
    float origin[3]{0.0f, 0.0f, 0.0f};
    float xAxis[3]{1.0f, 0.0f, 0.0f};
    float yAxis[3]{0.0f, 1.0f, 0.0f};
    std::vector<SketchSegment2D> segments;
};

struct HandSkeletonRenderData
{
    static constexpr int kMaxHands = 2;
    static constexpr int kMaxJoints = 21;

    int mode{0};

    int handCount{0};

    float positions[kMaxHands][kMaxJoints * 3]{};

    float confidence[kMaxHands][kMaxJoints]{};

    int handedness[kMaxHands]{0, 0};

    float pinch[kMaxHands]{0.0f, 0.0f};

    int gesture[kMaxHands]{0, 0};

    void clear() noexcept { mode = 0; handCount = 0; }
};

struct HandSkeletonStyle
{
    float leftColor[3]{0.20f, 0.90f, 0.95f};
    float rightColor[3]{1.00f, 0.55f, 0.15f};
    float jointSize{5.0f};
    float tipSize{7.0f};
    float wristSize{8.0f};
    float alpha{0.95f};
    bool depthTest{false};
};

class RenderContext {
public:

    Matrix4Raw viewMatrix{IdentityMatrix4()};

    Matrix4Raw projectionMatrix{IdentityMatrix4()};

    Matrix4Raw viewProjectionMatrix{IdentityMatrix4()};

    float cameraPosition[3]{0.0f, 0.0f, 0.0f};

    float nearPlane{0.01f};
    float farPlane{1000000.0f};

    float fovY{0.7853981633974483f};

    std::uint32_t viewportWidth{0};
    std::uint32_t viewportHeight{0};
    float aspectRatio{1.0f};

    float deltaTime{0.0f};
    float totalTime{0.0f};
    std::uint64_t frameNumber{0};

    const std::vector<std::uint64_t>* selectionIds{nullptr};

    std::uint64_t selectionObjectId{0};
    std::uint64_t selectionFaceId{0};

    std::uint64_t hoverObjectId{0};

    float cursorNDC[2]{0.0f, 0.0f};
    bool cursorActive{false};

    std::vector<PlaneRenderData> planes;

    std::vector<SketchRenderData> sketches;

    HandSkeletonRenderData handSkeleton;

    void beginFrame(float dt, std::uint32_t width, std::uint32_t height) noexcept
    {
        deltaTime = dt;
        totalTime += dt;
        viewportWidth = width;
        viewportHeight = height;
        aspectRatio = (height > 0)
            ? static_cast<float>(width) / static_cast<float>(height)
            : 1.0f;
        ++frameNumber;
    }

    void endFrame() noexcept
    {
    }

    void updateMatrices(const Matrix4Raw& view, const Matrix4Raw& projection) noexcept
    {
        viewMatrix = view;
        projectionMatrix = projection;
        viewProjectionMatrix = multiplyMatrices(projection, view);
    }

    void setCameraPosition(float x, float y, float z) noexcept
    {
        cameraPosition[0] = x;
        cameraPosition[1] = y;
        cameraPosition[2] = z;
    }

    void setClippingPlanes(float nearVal, float farVal) noexcept
    {
        nearPlane = nearVal;
        farPlane = farVal;
    }

    void setSelection(const std::vector<std::uint64_t>* ids) noexcept
    {
        selectionIds = ids;
    }

    void setSelectionFace(std::uint64_t objectId, std::uint64_t faceId) noexcept
    {
        selectionObjectId = objectId;
        selectionFaceId = faceId;
    }

    void setHover(std::uint64_t objectId) noexcept
    {
        hoverObjectId = objectId;
    }

    void setHandSkeleton(const HandSkeletonRenderData& data) noexcept
    {
        handSkeleton = data;
    }

    void reset() noexcept
    {
        viewMatrix = IdentityMatrix4();
        projectionMatrix = IdentityMatrix4();
        viewProjectionMatrix = IdentityMatrix4();
        cameraPosition[0] = cameraPosition[1] = cameraPosition[2] = 0.0f;
        nearPlane = 0.01f;
        farPlane = 1000000.0f;
        viewportWidth = 0;
        viewportHeight = 0;
        aspectRatio = 1.0f;
        deltaTime = 0.0f;
        totalTime = 0.0f;
        frameNumber = 0;
        selectionIds = nullptr;
        selectionObjectId = 0;
        selectionFaceId = 0;
        hoverObjectId = 0;
        handSkeleton.clear();
    }

private:

    static Matrix4Raw multiplyMatrices(const Matrix4Raw& a,
                                       const Matrix4Raw& b) noexcept
    {
        Matrix4Raw result{};
        for (int column = 0; column < 4; ++column)
        {
            for (int row = 0; row < 4; ++row)
            {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                {
                    sum += a[row + k * 4] * b[k + column * 4];
                }
                result[row + column * 4] = sum;
            }
        }
        return result;
    }
};

}