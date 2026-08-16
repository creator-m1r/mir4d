// MirEngine/Rendering/Core/RenderContext.h
// =================================================================================
// Per-frame rendering context.
//
// Carries everything that flows from the scene / camera into the renderer and
// is consumed by RenderPass implementations:
//
//   - camera matrices (view, projection, view-projection);
//   - camera parameters (position, clipping planes, FOV);
//   - viewport dimensions;
//   - frame timing (delta time, total time, frame number);
//   - current selection set (object ids to highlight).
//
// Key rule:
//   RenderContext is backend-neutral. It stores only mathematical and
//   geometric quantities. The renderer translates them into GPU calls.
//
// Life cycle:
//   1. beginFrame() at the start of the frame;
//   2. updateMatrices() / setCameraPosition() from the camera;
//   3. read by all RenderPass implementations;
//   4. endFrame() at the end of the frame.
// =================================================================================

#pragma once

#include <cstdint>
#include <vector>

#include "RenderCommand.h"

namespace MirEngine::Rendering {

// -----------------------------------------------------------------------------
// Work plane overlay data (ТЗ Этап 1, раздел 5).
// Backend-neutral: only mathematical/geometric quantities, no GPU types.
// -----------------------------------------------------------------------------
struct PlaneRenderData
{
    std::uint32_t id{0};
    float origin[3]{0.0f, 0.0f, 0.0f};
    float normal[3]{0.0f, 0.0f, 1.0f};
    float xAxis[3]{1.0f, 0.0f, 0.0f};
    float yAxis[3]{0.0f, 1.0f, 0.0f};
    float color[3]{0.45f, 0.55f, 0.75f};
    float size{10.0f};     // half-extent of the drawn working surface
    bool active{false};    // the plane a new sketch would attach to
    bool selected{false};  // currently selected in the UI
};

struct SketchSegment2D
{
    float ax{0.0f};        // local x of start point (plane frame)
    float ay{0.0f};        // local y of start point
    float bx{0.0f};        // local x of end point
    float by{0.0f};        // local y of end point
    float color[3]{0.95f, 0.85f, 0.25f}; // amber sketch stroke
};

// 2D-эскиз, заданный в локальной СК рабочей плоскости (ТЗ Этап 2).
struct SketchRenderData
{
    float origin[3]{0.0f, 0.0f, 0.0f};
    float xAxis[3]{1.0f, 0.0f, 0.0f};
    float yAxis[3]{0.0f, 1.0f, 0.0f};
    std::vector<SketchSegment2D> segments;
};


// -----------------------------------------------------------------------------
// Per-frame rendering context.
// -----------------------------------------------------------------------------
class RenderContext {
public:
    // ==========================================================================
    // Matrices
    // ==========================================================================

    // World -> camera
    Matrix4Raw viewMatrix{IdentityMatrix4()};

    // Camera -> clip
    Matrix4Raw projectionMatrix{IdentityMatrix4()};

    // Precomputed projection * view
    Matrix4Raw viewProjectionMatrix{IdentityMatrix4()};

    // ==========================================================================
    // Camera parameters
    // ==========================================================================

    // Camera position in world coordinates
    float cameraPosition[3]{0.0f, 0.0f, 0.0f};

    // Clipping planes.
    // CAD scenes are arbitrarily large: defaults cover millimeter parts and
    // kilometer assemblies. ViewportRuntime recomputes these every frame from
    // the scene bounds.
    float nearPlane{0.01f};
    float farPlane{1000000.0f};

    // Vertical field of view (radians)
    float fovY{0.7853981633974483f};

    // ==========================================================================
    // Viewport
    // ==========================================================================

    std::uint32_t viewportWidth{0};
    std::uint32_t viewportHeight{0};
    float aspectRatio{1.0f};

    // ==========================================================================
    // Frame timing
    // ==========================================================================

    float deltaTime{0.0f};    // seconds since the previous frame
    float totalTime{0.0f};    // total elapsed time (seconds)
    std::uint64_t frameNumber{0};

    // ==========================================================================
    // Selection
    // ==========================================================================

    // Object ids of the current selection set. The geometry pass uses this
    // set to highlight selected objects; empty set means no selection.
    const std::vector<std::uint64_t>* selectionIds{nullptr};

    // Face-level selection: when selectionObjectId is valid and
    // selectionFaceId is non-zero, only the triangles of that source B-Rep
    // face are highlighted (see TriangleMesh3::Triangle::sourceFaceId).
    std::uint64_t selectionObjectId{0};
    std::uint64_t selectionFaceId{0};

    // ==========================================================================
    // Hover
    // ==========================================================================

    // Object id currently under the cursor (hover). The geometry pass applies
    // a subtler tint than the selection highlight. Zero means no hover.
    std::uint64_t hoverObjectId{0};

    // ==========================================================================
    // Work planes (ТЗ Этап 1)
    // ==========================================================================

    // Planes to overlay in the viewport. Populated by the renderer from the
    // document's PlaneStore (via setPlanes). Empty when no planes are shown.
    std::vector<PlaneRenderData> planes;

    // 2D sketches overlaid on work planes (ТЗ Этап 2)
    std::vector<SketchRenderData> sketches;

    // ==========================================================================
    // Methods
    // ==========================================================================

    // Prepares the context for a new frame: updates sizes, timing and the
    // frame counter.
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

    // Finishes the frame (reserved for fences, queries, temporary data).
    void endFrame() noexcept
    {
    }

    // Updates the matrices and recomputes viewProjectionMatrix.
    void updateMatrices(const Matrix4Raw& view, const Matrix4Raw& projection) noexcept
    {
        viewMatrix = view;
        projectionMatrix = projection;
        viewProjectionMatrix = multiplyMatrices(projection, view);
    }

    // Sets the camera position.
    void setCameraPosition(float x, float y, float z) noexcept
    {
        cameraPosition[0] = x;
        cameraPosition[1] = y;
        cameraPosition[2] = z;
    }

    // Sets the clipping planes.
    void setClippingPlanes(float nearVal, float farVal) noexcept
    {
        nearPlane = nearVal;
        farPlane = farVal;
    }

    // Points the context at the current selection set (borrowed pointer).
    void setSelection(const std::vector<std::uint64_t>* ids) noexcept
    {
        selectionIds = ids;
    }

    // Selects a single source face of an object for highlight.
    void setSelectionFace(std::uint64_t objectId, std::uint64_t faceId) noexcept
    {
        selectionObjectId = objectId;
        selectionFaceId = faceId;
    }

    // Sets the object id under the cursor (hover highlight).
    void setHover(std::uint64_t objectId) noexcept
    {
        hoverObjectId = objectId;
    }

    // Resets every value to its default.
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
    }

private:
    // Column-major 4x4 matrix multiplication.
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

} // namespace MirEngine::Rendering