
#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace MirUI {

class GridManager;
class GuideManager;

class PreviewManager {
public:

    PreviewManager()
        : m_previewMode(false)
    {}

    void enterPreview() {
        if (m_previewMode) return;
        m_previewMode = true;

        setGridVisible(false);
        setGuidesVisible(false);

        for (auto& callback : m_onEnterCallbacks) {
            if (callback) callback();
        }
    }

    void exitPreview() {
        if (!m_previewMode) return;
        m_previewMode = false;

        setGridVisible(true);
        setGuidesVisible(true);

        for (auto& callback : m_onExitCallbacks) {
            if (callback) callback();
        }
    }

    void togglePreview() {
        if (m_previewMode) {
            exitPreview();
        } else {
            enterPreview();
        }
    }

    [[nodiscard]] bool isPreviewMode() const { return m_previewMode; }

    void onEnter(const std::function<void()>& callback) {
        m_onEnterCallbacks.push_back(callback);
    }
    void onExit(const std::function<void()>& callback) {
        m_onExitCallbacks.push_back(callback);
    }

    void attachGridManager(GridManager* grid) { m_grid = grid; }
    void attachGuideManager(GuideManager* guide) { m_guide = guide; }

private:
    bool m_previewMode;

    GridManager* m_grid = nullptr;
    GuideManager* m_guide = nullptr;

    bool m_savedGridVisible = true;
    bool m_savedGuidesVisible = true;

    std::vector<std::function<void()>> m_onEnterCallbacks;
    std::vector<std::function<void()>> m_onExitCallbacks;

    void setGridVisible(bool visible) {
        if (m_grid) {
            if (visible) {
                m_grid->setVisible(m_savedGridVisible);
            } else {
                m_savedGridVisible = m_grid->isVisible();
                m_grid->setVisible(false);
            }
        }
    }

    void setGuidesVisible(bool visible) {
        if (m_guide) {
            if (visible) {
                m_guide->setVisible(m_savedGuidesVisible);
            } else {
                m_savedGuidesVisible = m_guide->isVisible();
                m_guide->setVisible(false);
            }
        }
    }
};

}