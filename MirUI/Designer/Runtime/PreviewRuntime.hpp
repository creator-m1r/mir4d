
#pragma once

#include "../Model/UIProject.hpp"
#include "../../Core/Rendering/Renderer.hpp"
#include "../../Core/Layout/LayoutEngine.hpp"
#include <memory>

namespace MirUI {

class PreviewRuntime {
public:

    explicit PreviewRuntime(UIProject& project)
        : m_project(project)
        , m_renderer(nullptr)
        , m_previewMode(false)
    {}

    void setRenderer(Renderer* renderer) {
        m_renderer = renderer;
    }
    [[nodiscard]] Renderer* renderer() const { return m_renderer; }

    void enterPreview() {
        if (m_previewMode) return;
        m_previewMode = true;
        if (m_onEnterPreview) m_onEnterPreview();

        render();
    }

    void exitPreview() {
        if (!m_previewMode) return;
        m_previewMode = false;
        if (m_onExitPreview) m_onExitPreview();
    }

    void togglePreview() {
        if (m_previewMode) {
            exitPreview();
        } else {
            enterPreview();
        }
    }

    [[nodiscard]] bool isPreviewMode() const { return m_previewMode; }

    void setOnEnterPreview(std::function<void()> callback) { m_onEnterPreview = std::move(callback); }
    void setOnExitPreview(std::function<void()> callback)  { m_onExitPreview  = std::move(callback); }

    void render() {
        if (!m_renderer) return;

        LayoutEngine engine;
        engine.layout(m_project.widgetTree());

        m_renderer->beginFrame();
        m_renderer->render(m_project.widgetTree());
        m_renderer->endFrame();
    }

    void update() {
        if (m_previewMode) {
            render();
        }
    }

private:
    UIProject& m_project;
    Renderer*  m_renderer = nullptr;
    bool       m_previewMode = false;

    std::function<void()> m_onEnterPreview;
    std::function<void()> m_onExitPreview;
};

}