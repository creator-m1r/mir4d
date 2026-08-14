#include "RenderViewport.hpp"
#include "../RenderBounds.hpp"

#include <algorithm>
#include <utility>

namespace mir
{

bool RenderViewport::initialize() noexcept
{
    if (!renderer_.initialize())
        return false;

    if (!selectionRenderer_.initialize())
    {
        renderer_.destroy();
        return false;
    }

    camera_.setPerspective(1.0471975511965976, 1.0, 0.01, 10000.0);
    initialized_ = true;
    return true;
}

void RenderViewport::destroy() noexcept
{
    if (gpuMesh_.valid())
        uploader_.release(gpuMesh_);

    selection_.clear();
    selectionProperties_ = {};
    currentMesh_ = nullptr;
    selectionRenderer_.destroy();
    renderer_.destroy();
    initialized_ = false;
}

void RenderViewport::resize(RenderViewportSize size) noexcept
{
    size_.width = std::max(size.width, 1);
    size_.height = std::max(size.height, 1);

    const double aspect = static_cast<double>(size_.width) /
                          static_cast<double>(size_.height);
    camera_.setPerspective(1.0471975511965976, aspect, 0.01, 10000.0);
    renderer_.resize(size_.width, size_.height);
}

void RenderViewport::processInput(const RenderViewportInput& input) noexcept
{
    constexpr double orbitSensitivity = 0.008;
    constexpr double zoomSensitivity = 0.15;
    constexpr double panSensitivity = 0.01;

    camera_.setYaw(camera_.yaw() + input.orbitDeltaX * orbitSensitivity);
    camera_.setPitch(camera_.pitch() + input.orbitDeltaY * orbitSensitivity);

    const double zoomFactor = std::max(0.05, 1.0 - input.zoomDelta * zoomSensitivity);
    camera_.setDistance(std::max(1e-3, camera_.distance() * zoomFactor));

    if (input.panDeltaX != 0.0 || input.panDeltaY != 0.0)
    {
        camera_.moveTarget({
            -input.panDeltaX * panSensitivity,
            input.panDeltaY * panSensitivity,
            0.0});
    }

    if (input.fitAll && currentMesh_ != nullptr)
    {
        const auto bounds = RenderBounds::fromMesh(*currentMesh_);
        if (bounds.valid())
        {
            camera_.setTarget(bounds.center());
            camera_.setDistance(std::max(bounds.radius() * 2.5, 1e-3));
        }
    }
}

RenderRay RenderViewport::rayAt(double screenX, double screenY) const noexcept
{
    return RenderScreenRayBuilder::fromScreen(
        screenX, screenY,
        static_cast<double>(size_.width),
        static_cast<double>(size_.height),
        camera_);
}

void RenderViewport::rebuildSelectionProperties() noexcept
{
    if (currentMesh_ == nullptr)
    {
        selectionProperties_ = {};
        return;
    }

    selectionProperties_ = RenderSelectionPropertiesBuilder::build(
        selection_.selected(),
        *currentMesh_);
}

void RenderViewport::notifySelectionChanged() noexcept
{
    if (selectionChangedCallback_)
        selectionChangedCallback_(selection_.selected(), selectionProperties_);
}

void RenderViewport::hoverAt(double screenX, double screenY) noexcept
{
    if (!initialized_ || currentMesh_ == nullptr)
        return;

    selection_.hover(rayAt(screenX, screenY), *currentMesh_);
}

void RenderViewport::selectAt(double screenX, double screenY, bool additive) noexcept
{
    if (!initialized_ || currentMesh_ == nullptr)
        return;

    selection_.select(rayAt(screenX, screenY), *currentMesh_, additive);
    rebuildSelectionProperties();
    notifySelectionChanged();
}

void RenderViewport::clearSelection() noexcept
{
    selection_.clear();
    selectionProperties_ = {};
    notifySelectionChanged();
}

void RenderViewport::beginFrame() const noexcept
{
    renderer_.beginFrame();
}

void RenderViewport::draw(const RenderMesh& mesh) noexcept
{
    if (!initialized_ || mesh.empty())
        return;

    currentMesh_ = &mesh;

    if (gpuMesh_.valid())
        uploader_.release(gpuMesh_);

    gpuMesh_ = uploader_.upload(mesh);
    renderer_.draw(gpuMesh_, camera_);

    selectionRenderer_.draw(selection_.hoverOverlay(), mesh, camera_);
    selectionRenderer_.draw(selection_.selectedOverlay(), mesh, camera_);

    rebuildSelectionProperties();
}

void RenderViewport::endFrame() const noexcept
{
    renderer_.endFrame();
}

} // namespace mir
