#pragma once

#include "ViewportState.hpp"
#include "MirEngine/Geometry/Scene/Scene.hpp"
#include "MirEngine/Rendering/Renderer.h"
#include "MirEngine/Rendering/Core/RenderContext.h"

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

    [[nodiscard]] PickResult pick(Scalar x, Scalar y) const noexcept
    {
        if (!scene_)
            return {};
        return state_.pick(*scene_, x, y);
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
