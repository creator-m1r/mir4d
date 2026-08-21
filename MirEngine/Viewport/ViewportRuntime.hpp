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

    void clearSelection() noexcept
    {
        state_.selection.clear();
    }

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
        deformSnapshot_ = node->model()->mesh().vertices;
    }

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

    void updateHover(Scalar x, Scalar y) noexcept
    {
        if (!scene_)
        {
            state_.hoveredObjectId = mir4d::InvalidObjectId;
            return;
        }
        const PickResult result = state_.pick(*scene_, x, y);
        state_.hoveredObjectId =
            result.hit() ? result.objectId : mir4d::InvalidObjectId;
    }

    void clearHover() noexcept
    {
        state_.hoveredObjectId = mir4d::InvalidObjectId;
    }

    [[nodiscard]] mir4d::ObjectId hoveredObjectId() const noexcept
    {
        return state_.hoveredObjectId;
    }

    void setHandHover(mir4d::ObjectId objectId) noexcept
    {
        state_.hoveredObjectId = objectId;
    }

    void setHandSkeleton(const MirEngine::Rendering::HandSkeletonRenderData& data) noexcept
    {
        m_handSkeletonData = data;
    }

    void clearHandSkeleton() noexcept
    {
        m_handSkeletonData.clear();
    }

    void setHandSkeletonStyle(const MirEngine::Rendering::HandSkeletonStyle& style) noexcept
    {
        if (renderer_)
            renderer_->setHandSkeletonStyle(style);
    }

    void setHandSkeletonTopology(const std::vector<std::pair<int, int>>& bones) noexcept
    {
        if (renderer_)
            renderer_->setHandSkeletonTopology(bones);
    }

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

    void previewHandGrab(const Transform& transform) noexcept
    {
        if (!scene_ || !mir4d::isValidObjectId(handGrabId_))
            return;
        if (auto node = scene_->find(handGrabId_))
            node->setTransform(transform);
    }

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

    [[nodiscard]] Transform objectTransform(mir4d::ObjectId objectId) const noexcept
    {
        if (scene_)
            if (auto node = scene_->find(objectId))
                return node->transform();
        return Transform::identity();
    }

    [[nodiscard]] PickResult pickWorldRay(
        const Point3& origin, const Vector3& direction) const noexcept
    {
        if (!scene_)
            return {};
        return RayPicker::pick(*scene_, PickRay{origin, direction});
    }

    void setObjectMaterial(mir4d::ObjectId objectId,
                           MirEngine::Rendering::MaterialId materialId) noexcept
    {
        if (renderer_)
            renderer_->setObjectMaterial(
                static_cast<std::uint64_t>(objectId),
                static_cast<std::int32_t>(materialId));
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
            state_.selection.toggle(result.objectId);
        else if (result.faceId != 0)
            state_.selection.selectFace(result.objectId, result.faceId);
        else
            state_.selection.select(result.objectId, false);
        return true;
    }

    void update(double ) noexcept
    {

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

        const double farPlane = distanceToScene + sceneRadius +
                                std::max(sceneRadius * 0.5, distanceToScene * 0.1) + 1.0;
        const double nearPlane = std::clamp(distanceToScene * 0.0008, 0.0005, 5.0);

        const Scalar fov = state_.camera.fovY();
        const Scalar aspect = context.aspectRatio > 0
            ? Scalar(context.aspectRatio)
            : Scalar(1.0);

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

        if (state_.selection.faceId() != 0)
        {
            context.setSelectionFace(static_cast<std::uint64_t>(state_.selection.primary()),
                                     state_.selection.faceId());
        }
        else
        {
            context.setSelection(&state_.selection.ids());
        }

        context.setHover(static_cast<std::uint64_t>(state_.hoveredObjectId));

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

    mir4d::ObjectId dragCandidateId_{mir4d::InvalidObjectId};
    Scalar dragStartX_{0};
    Scalar dragStartY_{0};
    Transform dragStartTransform_{Transform::identity()};
    bool dragging_{false};

    mir4d::ObjectId handGrabId_{mir4d::InvalidObjectId};
    Transform handGrabStartTransform_{Transform::identity()};

    void resetHandGrab() noexcept
    {
        handGrabId_ = mir4d::InvalidObjectId;
        handGrabStartTransform_ = Transform::identity();
    }

    mir4d::ObjectId deformSnapshotId_{mir4d::InvalidObjectId};
    std::vector<mir::Point3> deformSnapshot_{};
};

}
