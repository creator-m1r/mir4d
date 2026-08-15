#pragma once

#include "ViewportState.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Rendering/Renderer.h"
#include "MirEngine/Rendering/Core/RenderContext.h"
#include "MirEngine/Rendering/Material/MaterialLibrary.hpp"

#include <algorithm>
#include <cstdint>

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

    [[nodiscard]] PickResult pick(Scalar x, Scalar y) const noexcept
    {
        if (!scene_)
            return {};
        return state_.pick(*scene_, x, y);
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
        else
            state_.selection.select(result.objectId, false);
        return true;
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

    MirEngine::Rendering::Renderer* renderer_{nullptr};
    Scene* scene_{nullptr};
    ViewportState state_{};
    MirEngine::Rendering::RenderContext renderContext_{};
};

} // namespace mir
