#pragma once

#include "RenderViewportInput.hpp"
#include "../OpenGL/OpenGLFrameRenderer.hpp"
#include "../OpenGL/OpenGLRenderMeshUploader.hpp"
#include "../Selection/RenderScreenRay.hpp"
#include "../Selection/RenderSelectionController.hpp"
#include "../Selection/RenderSelectionProperties.hpp"
#include "../Selection/OpenGLSelectionRenderer.hpp"

#include <functional>

namespace MirEngine {
namespace Rendering {

class RenderViewport
{
public:
    using SelectionChangedCallback = std::function<void(const RenderSelection&, const RenderSelectionProperties&)>;

    [[nodiscard]] bool initialize() noexcept;
    void destroy() noexcept;

    void resize(RenderViewportSize size) noexcept;
    void processInput(const RenderViewportInput& input) noexcept;

    void hoverAt(double screenX, double screenY) noexcept;
    void selectAt(double screenX, double screenY, bool additive = false) noexcept;
    void clearSelection() noexcept;

    void setSelectionChangedCallback(SelectionChangedCallback callback)
    {
        selectionChangedCallback_ = std::move(callback);
    }

    void beginFrame() const noexcept;
    void draw(const RenderMesh& mesh) noexcept;
    void endFrame() const noexcept;

    [[nodiscard]] const RenderCamera& camera() const noexcept { return camera_; }
    [[nodiscard]] RenderCamera& camera() noexcept { return camera_; }
    [[nodiscard]] const RenderSelectionController& selection() const noexcept { return selection_; }
    [[nodiscard]] const RenderSelectionProperties& selectionProperties() const noexcept { return selectionProperties_; }
    [[nodiscard]] bool valid() const noexcept { return initialized_; }

private:
    [[nodiscard]] RenderRay rayAt(double screenX, double screenY) const noexcept;
    void rebuildSelectionProperties() noexcept;
    void notifySelectionChanged() noexcept;

    bool initialized_{false};
    RenderViewportSize size_{};
    RenderCamera camera_{};
    OpenGLRenderMeshUploader uploader_{};
    OpenGLFrameRenderer renderer_{};
    OpenGLSelectionRenderer selectionRenderer_{};
    RenderSelectionController selection_{};
    RenderSelectionProperties selectionProperties_{};
    RenderMeshGPU gpuMesh_{};
    const RenderMesh* currentMesh_{nullptr};
    SelectionChangedCallback selectionChangedCallback_{};
};

} // namespace Rendering
} // namespace MirEngine
