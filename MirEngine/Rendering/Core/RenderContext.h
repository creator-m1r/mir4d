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
    bool hovered{false};   // under the cursor (hover highlight)
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
// Hand-skeleton visualization (debug / assist mode).
//
// Transient per-frame data pushed from the hand-tracking subsystem. It is a
// sensor view only: the pass that consumes it NEVER mutates the CAD scene,
// Document or command history. Copied into the context by the ViewportRuntime
// each frame; when the mode is Off (or no hands are tracked) handCount is 0 and
// the pass is a zero-cost no-op.
// -----------------------------------------------------------------------------
struct HandSkeletonRenderData
{
    static constexpr int kMaxHands = 2;
    static constexpr int kMaxJoints = 21;

    // Visibility mode (mirrors the Swift MIRHandSkeletonVisMode raw value):
    // 0 = off, 1 = jointsOnly, 2 = bones, 3 = bonesAndRays.
    int mode{0};

    // Number of valid hands in the arrays below (0 when off / no tracking).
    int handCount{0};

    // Per-hand joint positions (scene space, xyz) in LandmarkID.allCases order.
    float positions[kMaxHands][kMaxJoints * 3]{};
    // Per-hand per-joint tracking confidence in [0,1].
    float confidence[kMaxHands][kMaxJoints]{};
    // Per-hand handedness: 0 = left, 1 = right, 2 = none.
    int handedness[kMaxHands]{0, 0};
    // Per-hand pinch strength in [0,1] (drives tip accent + pinch line).
    float pinch[kMaxHands]{0.0f, 0.0f};
    // Per-hand active gesture code (index into MIRHandGestureType.allCases).
    int gesture[kMaxHands]{0, 0};

    void clear() noexcept { mode = 0; handCount = 0; }
};


// -----------------------------------------------------------------------------
// Style for the hand-skeleton overlay (debug / assist mode).
//
// Driven from the hand-tracking configuration (colours, sizes, transparency,
// depth behaviour). Kept separate from the per-frame `HandSkeletonRenderData`
// so it can be pushed once and reused across frames.
// -----------------------------------------------------------------------------
struct HandSkeletonStyle
{
    float leftColor[3]{0.20f, 0.90f, 0.95f};   // cyan
    float rightColor[3]{1.00f, 0.55f, 0.15f};  // orange
    float jointSize{5.0f};
    float tipSize{7.0f};
    float wristSize{8.0f};
    float alpha{0.95f};
    bool depthTest{false};
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

    // Cursor position in normalized device coordinates (x,y in [-1,1]) used to
    // pick the work plane under the pointer. cursorActive is false when the
    // pointer left the viewport.
    float cursorNDC[2]{0.0f, 0.0f};
    bool cursorActive{false};

    // ==========================================================================
    // Work planes (ТЗ Этап 1)
    // ==========================================================================

    // Planes to overlay in the viewport. Populated by the renderer from the
    // document's PlaneStore (via setPlanes). Empty when no planes are shown.
    std::vector<PlaneRenderData> planes;

    // 2D sketches overlaid on work planes (ТЗ Этап 2)
    std::vector<SketchRenderData> sketches;

    // Hand-skeleton overlay (debug / assist). Transient sensor view; never
    // feeds back into the CAD scene or undo history.
    HandSkeletonRenderData handSkeleton;

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

    // Copies the hand-skeleton overlay data for the frame (sensor view only).
    void setHandSkeleton(const HandSkeletonRenderData& data) noexcept
    {
        handSkeleton = data;
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
        handSkeleton.clear();
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