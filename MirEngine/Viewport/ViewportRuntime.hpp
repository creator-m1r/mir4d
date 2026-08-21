#pragma once

#include "ViewportState.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Rendering/Renderer.h"
#include "MirEngine/Rendering/Core/RenderContext.h"
#include "MirEngine/Rendering/Material/MaterialLibrary.hpp"
#include "MirEngine/Document/SceneCommandHistory.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace mir
{

/// Canonical runtime owner for one interactive viewport.
/// It owns only presentation/interaction state and references the engineering Scene.
/// Renderer remains responsible for GPU/backend work.
class ViewportRuntime
{
public:
    explicit ViewportRuntime(MirEngine::Rendering::Renderer* renderer = nullptr) noexcept
        : renderer_(renderer)
    {
    }

    void setRenderer(MirEngine::Rendering::Renderer* renderer) noexcept
    {
        renderer_ = renderer;
    }

    void setScene(Scene* scene) noexcept
    {
        scene_ = scene;
        state_.selection.clear();
        state_.hoveredObjectId = mir4d::InvalidObjectId;
        history_.clear();
    }

    [[nodiscard]] Scene* scene() const noexcept { return scene_; }
    [[nodiscard]] ViewportState& state() noexcept { return state_; }
    [[nodiscard]] const ViewportState& state() const noexcept { return state_; }

    void resize(std::uint32_t width, std::uint32_t height) noexcept
    {
        state_.resize(width, height);
        if (renderer_)
            renderer_->resize(state_.width, state_.height);
    }

    void beginOrbit(Scalar x, Scalar y) noexcept { state_.controller.beginOrbit(x, y); }
    void beginPan(Scalar x, Scalar y) noexcept { state_.controller.beginPan(x, y); }
    void endInteraction() noexcept { state_.controller.end(); }
    void move(Scalar x, Scalar y) noexcept { state_.controller.move(x, y); }
    void zoom(Scalar delta) noexcept { state_.controller.zoom(delta); }
    void panBy(Scalar dx, Scalar dy) noexcept { state_.controller.panBy(dx, dy); }
    void orbitBy(Scalar dx, Scalar dy) noexcept { state_.controller.orbitBy(dx, dy); }

    // ── Manipulation: Hover → Selection → Drag ─────────────────────────
    //
    // Left-button down on an already selected object arms a move-drag
    // candidate; the camera stays locked until the drag threshold is crossed
    // (a plain click keeps selection untouched and lets the UI run its own
    // pick). Left-button down on empty space or an unselected object falls
    // back to orbit. Middle/right buttons keep their pan behavior.

    void handleMouseDown(int button, Scalar x, Scalar y) noexcept
    {
        state_.controller.end();
        dragging_ = false;
        dragCandidateId_ = mir4d::InvalidObjectId;

        constexpr int leftButton = 0;
        if (button == leftButton)
        {
            const PickResult result = pick(x, y);
            if (result.hit() &&
                scene_ &&
                state_.selection.contains(result.objectId))
            {
                dragCandidateId_ = result.objectId;
                dragStartX_ = x;
                dragStartY_ = y;
                const auto node = scene_->find(result.objectId);
                dragStartTransform_ =
                    node ? node->transform() : Transform::identity();
                return;
            }
            state_.controller.beginOrbit(x, y);
            return;
        }

        state_.controller.beginPan(x, y);
    }

    void handleMouseMove(Scalar x, Scalar y) noexcept
    {
        if (dragging_)
        {
            updateDrag(x, y);
            return;
        }

        if (mir4d::isValidObjectId(dragCandidateId_))
        {
            const Scalar dx = x - dragStartX_;
            const Scalar dy = y - dragStartY_;
            constexpr Scalar kDragThresholdSquared = Scalar(6) * Scalar(6);
            if (dx * dx + dy * dy > kDragThresholdSquared)
            {
                dragging_ = true;
                updateDrag(x, y);
            }
            return;
        }

        state_.controller.move(x, y);
    }

    void handleMouseUp(int button, Scalar x, Scalar y) noexcept
    {
        (void)button;
        (void)x;
        (void)y;

        // A finished drag commits exactly one Move operation; a drag over N
        // frames never produces N history entries.
        if (dragging_ && scene_ &&
            mir4d::isValidObjectId(dragCandidateId_))
        {
            const auto node = scene_->find(dragCandidateId_);
            if (node && node->transform() != dragStartTransform_)
            {
                history_.execute(
                    std::make_unique<mir4d::MoveObjectCommand>(
                        dragCandidateId_,
                        dragStartTransform_,
                        node->transform()),
                    *scene_);
            }
        }

        dragging_ = false;
        dragCandidateId_ = mir4d::InvalidObjectId;
        state_.controller.end();
    }

    /// Aborts an active drag and restores the transform captured at drag start
    /// (Esc during manipulation). No history entry is created.
    void cancelDrag() noexcept
    {
        if (!scene_ || !mir4d::isValidObjectId(dragCandidateId_))
        {
            dragging_ = false;
            dragCandidateId_ = mir4d::InvalidObjectId;
            state_.controller.end();
            return;
        }

        if (auto node = scene_->find(dragCandidateId_))
            node->setTransform(dragStartTransform_);
        dragging_ = false;
        dragCandidateId_ = mir4d::InvalidObjectId;
        state_.controller.end();
    }

    [[nodiscard]] bool isDragging() const noexcept { return dragging_; }

    /// Removes the primary selection from the scene through the command
    /// history (undoable). MirEngine Scene is the single source of truth;
    /// the renderer just observes the changed scene.
    bool deleteSelectedObject() noexcept
    {
        if (!scene_)
            return false;

        const mir4d::ObjectId primary = state_.selection.primary();
        if (!mir4d::isValidObjectId(primary))
            return false;

        const auto node = scene_->find(primary);
        if (!node)
            return false;

        history_.execute(
            std::make_unique<mir4d::DeleteObjectCommand>(node),
            *scene_);

        state_.selection.deselect(primary);
        if (state_.hoveredObjectId == primary)
            state_.hoveredObjectId = mir4d::InvalidObjectId;
        return true;
    }

    /// Clears the selection set without touching the scene.
    void clearSelection() noexcept
    {
        state_.selection.clear();
    }

    // ── Undo / Redo ─────────────────────────────────────────────────────

    bool undo() noexcept
    {
        if (!scene_ || !history_.undo(*scene_))
            return false;
        state_.selection.clear();
        state_.hoveredObjectId = mir4d::InvalidObjectId;
        return true;
    }

    bool redo() noexcept
    {
        if (!scene_ || !history_.redo(*scene_))
            return false;
        state_.selection.clear();
        state_.hoveredObjectId = mir4d::InvalidObjectId;
        return true;
    }

    [[nodiscard]] bool canUndo() const noexcept { return history_.canUndo(); }
    [[nodiscard]] bool canRedo() const noexcept { return history_.canRedo(); }

    // ── Sculpt stroke (undoable) ─────────────────────────────────────────
    //
    // A hand sculpt stroke produces many deform frames; they must collapse into
    // exactly one undo entry. `beginDeformSelected` snapshots the selected mesh,
    // `MirEngineDeformSelected` mutates it live (no history), and `endDeformSelected`
    // commits a single `DeformObjectCommand` capturing before/after vertices.

    /// Snapshots the primary-selected mesh so a subsequent stroke is undoable.
    /// Safe to call when nothing is selected (no-op until `endDeformSelected`).
    void beginDeformSelected() noexcept
    {
        deformSnapshotId_ = mir4d::InvalidObjectId;
        deformSnapshot_.clear();
        if (!scene_)
            return;
        const mir4d::ObjectId id = state_.selection.primary();
        if (!mir4d::isValidObjectId(id))
            return;
        const auto node = scene_->find(id);
        if (!node || !node->model() || !node->model()->hasMesh())
            return;
        deformSnapshotId_ = id;
        deformSnapshot_ = node->model()->mesh().vertices; // copy
    }

    /// Commits one undoable deform command for the active stroke (if changed).
    void endDeformSelected() noexcept
    {
        if (!scene_ || !mir4d::isValidObjectId(deformSnapshotId_))
        {
            deformSnapshot_.clear();
            deformSnapshotId_ = mir4d::InvalidObjectId;
            return;
        }
        const auto node = scene_->find(deformSnapshotId_);
        const mir4d::ObjectId id = deformSnapshotId_;
        if (node && node->model() && node->model()->hasMesh())
        {
            const auto& mesh = node->model()->mesh();
            bool changed = mesh.vertices.size() == deformSnapshot_.size();
            if (changed)
            {
                for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
                {
                    const auto& a = mesh.vertices[i];
                    const auto& b = deformSnapshot_[i];
                    if (a.x != b.x || a.y != b.y || a.z != b.z)
                    {
                        changed = true;
                        break;
                    }
                }
            }
            if (changed)
            {
                history_.execute(
                    std::make_unique<mir4d::DeformObjectCommand>(
                        id, deformSnapshot_, mesh.vertices),
                    *scene_);
            }
        }
        deformSnapshot_.clear();
        deformSnapshotId_ = mir4d::InvalidObjectId;
    }

    void setProjection(CameraProjection projection) noexcept
    {
        state_.camera.setProjection(projection);
    }

    /// Zoom anchored at the cursor pixel (industrial CAD zoom-to-cursor).
    void zoomAt(Scalar delta, Scalar x, Scalar y) noexcept
    {
        const PickRay ray = RayPicker::buildRay(
            state_.camera, x, y, state_.width, state_.height);
        if (ray.direction.isZero())
        {
            state_.controller.zoom(delta);
            return;
        }
        state_.controller.zoomAt(delta, ray.origin, ray.direction);
    }

    [[nodiscard]] PickResult pick(Scalar x, Scalar y) const noexcept
    {
        if (!scene_)
            return {};
        return state_.pick(*scene_, x, y);
    }

    /// Recomputes the hovered object from the cursor position.
    /// Hover is purely presentational: it never mutates selection.
    ///
    /// Stabilization:
    ///  - throttle: the pick is only recomputed once the cursor has moved at
    ///    least `kHoverThrottlePx` since the last pick (avoids recomputing on
    ///    every mouse-moved pixel and reduces cost on dense meshes);
    ///  - hysteresis: a switch to a *different* element is only committed once
    ///    the cursor has moved at least `kHoverHysteresisPx` away from where the
    ///    current hover was established, preventing flicker on face/edge borders.
    void updateHover(Scalar x, Scalar y) noexcept
    {
        if (!scene_)
        {
            state_.hoveredObjectId = mir4d::InvalidObjectId;
            state_.hoveredKind = PickKind::Body;
            state_.hoveredElementId = 0;
            return;
        }

        constexpr Scalar kHoverThrottlePx = 1.0f;
        constexpr Scalar kHoverHysteresisPx = 8.0f;

        // Throttle: keep the current hover until the cursor moves enough.
        if (state_.hoveredObjectId != mir4d::InvalidObjectId)
        {
            const Scalar dx = x - state_.lastHoverPickX;
            const Scalar dy = y - state_.lastHoverPickY;
            if (dx * dx + dy * dy < kHoverThrottlePx * kHoverThrottlePx)
                return;
        }
        state_.lastHoverPickX = x;
        state_.lastHoverPickY = y;

        const PickResult result = state_.pick(*scene_, x, y);

        if (!result.hit())
        {
            state_.hoveredObjectId = mir4d::InvalidObjectId;
            state_.hoveredKind = PickKind::Body;
            state_.hoveredElementId = 0;
            return;
        }

        const bool changed =
            result.objectId != state_.hoveredObjectId ||
            result.kind != state_.hoveredKind ||
            result.elementId != state_.hoveredElementId;

        if (changed && state_.hoveredObjectId != mir4d::InvalidObjectId)
        {
            const Scalar dx = x - state_.lastHoverChangeX;
            const Scalar dy = y - state_.lastHoverChangeY;
            if (dx * dx + dy * dy < kHoverHysteresisPx * kHoverHysteresisPx)
                return; // keep current hover until the cursor moves away
        }

        state_.hoveredObjectId = result.objectId;
        state_.hoveredKind = result.kind;
        state_.hoveredElementId = result.elementId;
        state_.lastHoverChangeX = x;
        state_.lastHoverChangeY = y;
    }

    void clearHover() noexcept
    {
        state_.hoveredObjectId = mir4d::InvalidObjectId;
        state_.hoveredKind = PickKind::Body;
        state_.hoveredElementId = 0;
        state_.lastHoverChangeX = 0;
        state_.lastHoverChangeY = 0;
    }

    [[nodiscard]] mir4d::ObjectId hoveredObjectId() const noexcept
    {
        return state_.hoveredObjectId;
    }

    /// Highlights an object under the hand without changing the selection set.
    /// Consumed by the renderer's hover pass exactly like cursor hover.
    void setHandHover(mir4d::ObjectId objectId) noexcept
    {
        state_.hoveredObjectId = objectId;
        state_.hoveredKind = PickKind::Body;
        state_.hoveredElementId = 0;
    }

    /// Pushes the transient hand-skeleton overlay data for the current frame
    /// (debug / assist sensor view). The renderer draws it without ever
    /// mutating the scene, Document or command history.
    void setHandSkeleton(const MirEngine::Rendering::HandSkeletonRenderData& data) noexcept
    {
        m_handSkeletonData = data;
    }

    /// Clears the hand-skeleton overlay (no hands tracked / mode off).
    void clearHandSkeleton() noexcept
    {
        m_handSkeletonData.clear();
    }

    /// Forwards the hand-skeleton overlay style to the renderer (colours /
    /// sizes / depth behaviour). Single source of truth is the configuration.
    void setHandSkeletonStyle(const MirEngine::Rendering::HandSkeletonStyle& style) noexcept
    {
        if (renderer_)
            renderer_->setHandSkeletonStyle(style);
    }

    /// Forwards the hand-skeleton bone topology to the renderer.
    void setHandSkeletonTopology(const std::vector<std::pair<int, int>>& bones) noexcept
    {
        if (renderer_)
            renderer_->setHandSkeletonTopology(bones);
    }

    // ── Hand Grab (Vertical Slice v0.1) ───────────────────────────────────
    //
    // Mirrors the mouse-drag contract (see handleMouse*/updateDrag): a grab
    // captures the object's transform once, `previewHandGrab` mutates it live
    // with NO history, and exactly one `MoveObjectCommand` is committed on
    // release. State is kept separate from the mouse drag so the two input
    // methods never interfere. A lost-tracking grace period is handled by the
    // Swift controller, which calls `cancelHandGrab` on timeout.

    /// Arms a grab on `objectId`, snapshots its transform and selects it.
    void beginHandGrab(mir4d::ObjectId objectId) noexcept
    {
        if (!scene_ || !mir4d::isValidObjectId(objectId))
        {
            resetHandGrab();
            return;
        }
        const auto node = scene_->find(objectId);
        handGrabId_ = objectId;
        handGrabStartTransform_ = node ? node->transform() : Transform::identity();
        state_.selection.select(objectId, false);
    }

    /// Live preview of the grabbed object's transform. Never touches history.
    void previewHandGrab(const Transform& transform) noexcept
    {
        if (!scene_ || !mir4d::isValidObjectId(handGrabId_))
            return;
        if (auto node = scene_->find(handGrabId_))
            node->setTransform(transform);
    }

    /// Commits exactly one `MoveObjectCommand(initial, final)` for the grab.
    void commitHandGrab() noexcept
    {
        if (scene_ && mir4d::isValidObjectId(handGrabId_))
        {
            const auto node = scene_->find(handGrabId_);
            if (node && node->transform() != handGrabStartTransform_)
                history_.execute(
                    std::make_unique<mir4d::GrabTransformCommand>(
                        handGrabId_, handGrabStartTransform_, node->transform()),
                    *scene_);
        }
        resetHandGrab();
    }

    /// Cancels the active grab and restores the snapshot transform (no history).
    void cancelHandGrab() noexcept
    {
        if (scene_ && mir4d::isValidObjectId(handGrabId_))
        {
            if (auto node = scene_->find(handGrabId_))
                node->setTransform(handGrabStartTransform_);
        }
        resetHandGrab();
    }

    [[nodiscard]] bool isHandGrabbing() const noexcept
    {
        return mir4d::isValidObjectId(handGrabId_);
    }

    /// Current world transform of an object (seed for preview deltas).
    [[nodiscard]] Transform objectTransform(mir4d::ObjectId objectId) const noexcept
    {
        if (scene_)
            if (auto node = scene_->find(objectId))
                return node->transform();
        return Transform::identity();
    }

    /// Picks against an explicit world-space ray (hand tracking input).
    /// Uses the active selection filter so spatial input honours the mode.
    [[nodiscard]] PickResult pickWorldRay(
        const Point3& origin, const Vector3& direction) const noexcept
    {
        if (!scene_)
            return {};
        return RayPicker::pick(*scene_, PickRay{origin, direction}, state_.pickFilter);
    }

    /// Assigns a MaterialLibrary material id to an object.
    void setObjectMaterial(mir4d::ObjectId objectId,
                           MirEngine::Rendering::MaterialId materialId) noexcept
    {
        if (renderer_)
            renderer_->setObjectMaterial(
                static_cast<std::uint64_t>(objectId),
                static_cast<std::int32_t>(materialId));
    }

    void setPickFilter(PickFilter filter) noexcept
    {
        state_.setPickFilter(filter);
    }

    bool selectAt(Scalar x, Scalar y, bool additive = false) noexcept
    {
        const PickResult result = pick(x, y);
        if (!result.hit() || !mir4d::isValidObjectId(result.objectId))
        {
            if (!additive)
                state_.selection.clear();
            return false;
        }

        if (additive)
            state_.selection.toggleElement(result.kind, result.objectId, result.elementId);
        else
            state_.selection.selectElement(result.kind, result.objectId, result.elementId);
        return true;
    }

    /// Selects all objects whose projected bounds intersect the screen-space
    /// rectangle. See ViewportState::selectInRect for the picking semantics.
    bool selectInRect(Scalar x0, Scalar y0, Scalar x1, Scalar y1, bool additive = false) noexcept
    {
        if (!scene_)
        {
            if (!additive)
                state_.selection.clear();
            return false;
        }
        state_.selectInRect(*scene_, x0, y0, x1, y1, additive);
        return !state_.multiSelection.empty();
    }

    void update(double /*deltaTime*/) noexcept
    {
        // Camera controller is immediate-input driven.
        // This hook remains the canonical frame update boundary for future inertia.
    }

    void render()
    {
        if (!renderer_ || !scene_)
            return;

        auto& context = renderContext_;
        context.viewportWidth = state_.width;
        context.viewportHeight = state_.height;
        context.aspectRatio = state_.height > 0
            ? static_cast<float>(state_.width) / static_cast<float>(state_.height)
            : 1.0f;

        // Dynamic clipping: near/far planes follow the scene bounds and the
        // camera distance, so 1 mm parts and 10 km assemblies both render
        // without fixed-range clipping (MIR4D_RENDERING_VISUALIZATION spec).
        double minX = 0.0, minY = 0.0, minZ = 0.0;
        double maxX = 0.0, maxY = 0.0, maxZ = 0.0;
        bool hasBounds = false;
        for (const auto& node : scene_->nodes())
        {
            if (!node || !node->model() || !node->model()->hasMesh())
                continue;
            const auto& mesh = node->model()->mesh();
            if (mesh.empty())
                continue;

            const Matrix4 m = node->transform().matrix();
            for (const auto& vertex : mesh.vertices)
            {
                const double x = static_cast<double>(m(0, 0)) * vertex.x +
                                 static_cast<double>(m(0, 1)) * vertex.y +
                                 static_cast<double>(m(0, 2)) * vertex.z +
                                 static_cast<double>(m(0, 3));
                const double y = static_cast<double>(m(1, 0)) * vertex.x +
                                 static_cast<double>(m(1, 1)) * vertex.y +
                                 static_cast<double>(m(1, 2)) * vertex.z +
                                 static_cast<double>(m(1, 3));
                const double z = static_cast<double>(m(2, 0)) * vertex.x +
                                 static_cast<double>(m(2, 1)) * vertex.y +
                                 static_cast<double>(m(2, 2)) * vertex.z +
                                 static_cast<double>(m(2, 3));
                if (!hasBounds)
                {
                    minX = maxX = x;
                    minY = maxY = y;
                    minZ = maxZ = z;
                    hasBounds = true;
                }
                else
                {
                    minX = std::min(minX, x); maxX = std::max(maxX, x);
                    minY = std::min(minY, y); maxY = std::max(maxY, y);
                    minZ = std::min(minZ, z); maxZ = std::max(maxZ, z);
                }
            }
        }

        Point3 sceneCenter{0.0, 0.0, 0.0};
        Scalar sceneRadius = Scalar(1.0);
        if (hasBounds)
        {
            sceneCenter = Point3{
                (minX + maxX) * 0.5,
                (minY + maxY) * 0.5,
                (minZ + maxZ) * 0.5};
            const double dx = (maxX - minX) * 0.5;
            const double dy = (maxY - minY) * 0.5;
            const double dz = (maxZ - minZ) * 0.5;
            sceneRadius = std::max(std::sqrt(dx * dx + dy * dy + dz * dz), Scalar(1e-9));
        }

        const Point3 cameraPosition = state_.camera.position();
        const double dxc = sceneCenter.x - cameraPosition.x;
        const double dyc = sceneCenter.y - cameraPosition.y;
        const double dzc = sceneCenter.z - cameraPosition.z;
        const double distanceToScene = std::sqrt(dxc * dxc + dyc * dyc + dzc * dzc);

        // Far: distance + radius + adaptive margin; near: small, but never a
        // degenerate epsilon (avoids z-fighting, keeps 1 mm details visible).
        const double farPlane = distanceToScene + sceneRadius +
                                std::max(sceneRadius * 0.5, distanceToScene * 0.1) + 1.0;
        const double nearPlane = std::clamp(distanceToScene * 0.0008, 0.0005, 5.0);

        const Scalar fov = state_.camera.fovY();
        const Scalar aspect = context.aspectRatio > 0
            ? Scalar(context.aspectRatio)
            : Scalar(1.0);

        // Keep the active projection mode: perspective updates the FOV-based
        // matrix, orthographic only refreshes clip planes and aspect (extents
        // are derived from the orbit distance inside the camera).
        state_.camera.setAspect(aspect);
        state_.camera.setNearFar(nearPlane, farPlane);
        if (state_.camera.projection() == CameraProjection::Perspective)
            state_.camera.setPerspective(fov, aspect, nearPlane, farPlane);

        context.fovY = static_cast<float>(fov);
        context.nearPlane = static_cast<float>(nearPlane);
        context.farPlane = static_cast<float>(farPlane);

        const Matrix4 view = state_.camera.viewMatrix();
        const Matrix4 projection = state_.camera.projectionMatrix();
        context.updateMatrices(toRaw(view), toRaw(projection));

        const Point3 position = state_.camera.position();
        context.setCameraPosition(static_cast<float>(position.x),
                                  static_cast<float>(position.y),
                                  static_cast<float>(position.z));

        // Selection highlight set for the geometry pass. Sub-object selection
        // (face / edge / vertex) highlights only that element; the whole object
        // still receives a subtle selection tint. A box (rectangle) selection
        // populates state_.multiSelection, which is highlighted as a set of
        // bodies; in that case the primary selection mirrors the first hit.
        const auto& sel = state_.selection;
        const bool hasBoxSelection = !state_.multiSelection.empty();
        std::vector<mir4d::ObjectId> boxHighlightIds;
        if (hasBoxSelection)
        {
            boxHighlightIds.reserve(state_.multiSelection.size());
            for (const auto& entry : state_.multiSelection)
                boxHighlightIds.push_back(entry.id);
        }
        const auto& highlightSet = hasBoxSelection ? boxHighlightIds : sel.ids();
        const auto primaryId =
            static_cast<std::uint64_t>(sel.primary());

        context.setSelection(&highlightSet);

        if (sel.kind() == PickKind::Face && sel.elementId() != 0)
        {
            context.setSelectionFace(primaryId, sel.elementId());
        }
        else if (sel.kind() == PickKind::Edge)
        {
            context.setSelectionEdge(primaryId, sel.elementId());
        }
        else if (sel.kind() == PickKind::Vertex)
        {
            context.setSelectionVertex(primaryId, sel.elementId());
        }

        if (state_.hoveredObjectId != mir4d::InvalidObjectId)
        {
            if (state_.hoveredKind == PickKind::Edge)
                context.setHoverEdge(
                    static_cast<std::uint64_t>(state_.hoveredObjectId),
                    state_.hoveredElementId);
            else if (state_.hoveredKind == PickKind::Vertex)
                context.setHoverVertex(
                    static_cast<std::uint64_t>(state_.hoveredObjectId),
                    state_.hoveredElementId);
            else
                context.setHover(
                    static_cast<std::uint64_t>(state_.hoveredObjectId));
        }

        // Hand-skeleton overlay is a transient sensor view; copy it into the
        // per-frame context for the renderer's HandSkeletonPass.
        context.setHandSkeleton(m_handSkeletonData);

        renderer_->render(*scene_, context);
    }

private:
    static MirEngine::Rendering::Matrix4Raw toRaw(const Matrix4& matrix) noexcept
    {
        MirEngine::Rendering::Matrix4Raw result{};
        for (std::size_t row = 0; row < 4; ++row)
            for (std::size_t column = 0; column < 4; ++column)
                result[row + column * 4] = static_cast<float>(matrix(row, column));
        return result;
    }

    /// Projects the cursor ray onto the horizontal work plane (Z = planeZ)
    /// used by the active drag. The ground plane is XY for the Z-up camera.
    [[nodiscard]] static bool rayPlaneIntersection(const PickRay& ray,
                                                   Scalar planeZ,
                                                   Point3& hit) noexcept
    {
        if (std::abs(ray.direction.z) <= Scalar(1e-9))
            return false;
        const Scalar t = (planeZ - ray.origin.z) / ray.direction.z;
        if (t < Scalar(0))
            return false;
        hit = Point3{
            ray.origin.x + ray.direction.x * t,
            ray.origin.y + ray.direction.y * t,
            planeZ};
        return true;
    }

    /// Moves the drag candidate on the work plane. The delta is computed
    /// against the drag-start ray projection, so errors never accumulate
    /// frame over frame: current transform = dragStartTransform + delta.
    void updateDrag(Scalar x, Scalar y) noexcept
    {
        if (!scene_)
        {
            dragging_ = false;
            dragCandidateId_ = mir4d::InvalidObjectId;
            return;
        }

        auto node = scene_->find(dragCandidateId_);
        if (!node)
        {
            dragging_ = false;
            dragCandidateId_ = mir4d::InvalidObjectId;
            return;
        }

        const PickRay currentRay = RayPicker::buildRay(
            state_.camera, x, y, state_.width, state_.height);
        const PickRay startRay = RayPicker::buildRay(
            state_.camera, dragStartX_, dragStartY_, state_.width, state_.height);
        if (currentRay.direction.isZero() || startRay.direction.isZero())
            return;

        const Scalar planeZ = dragStartTransform_.position.z;
        Point3 currentHit{};
        Point3 startHit{};
        if (!rayPlaneIntersection(currentRay, planeZ, currentHit) ||
            !rayPlaneIntersection(startRay, planeZ, startHit))
            return;

        Transform updated = dragStartTransform_;
        updated.position.x += currentHit.x - startHit.x;
        updated.position.y += currentHit.y - startHit.y;
        updated.position.z += currentHit.z - startHit.z;
        node->setTransform(updated);
    }

    MirEngine::Rendering::Renderer* renderer_{nullptr};
    Scene* scene_{nullptr};
    ViewportState state_{};
    MirEngine::Rendering::RenderContext renderContext_{};
    MirEngine::Rendering::HandSkeletonRenderData m_handSkeletonData{};
    mir4d::SceneCommandHistory history_{};

    // Move-manipulation state. dragStartTransform_ keeps the transform
    // captured when the drag was armed, so Esc restores it exactly.
    mir4d::ObjectId dragCandidateId_{mir4d::InvalidObjectId};
    Scalar dragStartX_{0};
    Scalar dragStartY_{0};
    Transform dragStartTransform_{Transform::identity()};
    bool dragging_{false};

    // Hand-grab state (Vertical Slice v0.1). Independent from the mouse drag
    // so simultaneous inputs do not corrupt each other's snapshot.
    mir4d::ObjectId handGrabId_{mir4d::InvalidObjectId};
    Transform handGrabStartTransform_{Transform::identity()};

    void resetHandGrab() noexcept
    {
        handGrabId_ = mir4d::InvalidObjectId;
        handGrabStartTransform_ = Transform::identity();
    }

    // Sculpt-stroke snapshot (before-state for a single undoable DeformObjectCommand).
    mir4d::ObjectId deformSnapshotId_{mir4d::InvalidObjectId};
    std::vector<mir::Point3> deformSnapshot_{};
};

} // namespace mir
