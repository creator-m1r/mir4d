
#pragma once

#include "../Document/UIDocument.hpp"
#include "../Canvas/DesignerCanvas.hpp"
#include "../Toolbox/Toolbox.hpp"
#include "../Inspector/InspectorModel.hpp"
#include "../History/DesignerHistory.hpp"
#include "../Preview/PreviewManager.hpp"
#include <memory>
#include <string>

namespace MirUI {

class DesignerApplication {
public:

    DesignerApplication()
        : m_document(std::make_unique<UIDocument>())
        , m_history(std::make_unique<DesignerHistory>(*m_document))
        , m_toolbox(std::make_unique<Toolbox>(*m_document))
        , m_inspector(std::make_unique<InspectorModel>(*m_document))
        , m_canvas(std::make_unique<DesignerCanvas>(*m_document))
        , m_preview(std::make_unique<PreviewManager>())
    {

        newDocument();
    }

    [[nodiscard]] UIDocument& document() { return *m_document; }
    [[nodiscard]] const UIDocument& document() const { return *m_document; }

    [[nodiscard]] DesignerCanvas& canvas() { return *m_canvas; }
    [[nodiscard]] const DesignerCanvas& canvas() const { return *m_canvas; }

    [[nodiscard]] Toolbox& toolbox() { return *m_toolbox; }
    [[nodiscard]] const Toolbox& toolbox() const { return *m_toolbox; }

    [[nodiscard]] InspectorModel& inspector() { return *m_inspector; }
    [[nodiscard]] const InspectorModel& inspector() const { return *m_inspector; }

    [[nodiscard]] DesignerHistory& history() { return *m_history; }
    [[nodiscard]] const DesignerHistory& history() const { return *m_history; }

    [[nodiscard]] PreviewManager& preview() { return *m_preview; }
    [[nodiscard]] const PreviewManager& preview() const { return *m_preview; }

    void newDocument() {
        m_document->clear();

        auto window = WidgetFactory::create(WidgetType::Window);
        if (window) {
            window->setName("MainWindow");
            window->setLayoutData(LayoutData::fixed(1024, 768));
            m_document->widgetTree().setRoot(std::move(window));
        }

        m_history->clear();
        m_inspector->inspectWidget(std::nullopt);
        m_preview->exitPreview();
    }

    bool open(const std::string& path) {
        UIReader reader;
        UIDocument tempDoc;
        if (!reader.load(path, tempDoc)) {
            return false;
        }

        *m_document = std::move(tempDoc);
        m_history->clear();
        m_inspector->inspectWidget(std::nullopt);
        m_preview->exitPreview();
        return true;
    }

    bool save() {
        const std::string& path = m_document->filePath();
        if (path.empty()) {
            return false;
        }
        UIWriter writer;
        if (writer.save(path, *m_document)) {
            m_history->markSaved();
            return true;
        }
        return false;
    }

    bool saveAs(const std::string& path) {
        UIWriter writer;
        if (writer.save(path, *m_document)) {
            m_history->markSaved();
            return true;
        }
        return false;
    }

    void undo() {
        m_history->undo();

        m_inspector->inspectWidget(m_document->selection().singleSelected());
    }

    void redo() {
        m_history->redo();
        m_inspector->inspectWidget(m_document->selection().singleSelected());
    }

    void enterPreview() {
        m_preview->enterPreview();
    }

    void exitPreview() {
        m_preview->exitPreview();
    }

    void togglePreview() {
        m_preview->togglePreview();
    }

    [[nodiscard]] bool isPreviewMode() const {
        return m_preview->isPreviewMode();
    }

private:
    std::unique_ptr<UIDocument>        m_document;
    std::unique_ptr<DesignerHistory>   m_history;
    std::unique_ptr<Toolbox>           m_toolbox;
    std::unique_ptr<InspectorModel>    m_inspector;
    std::unique_ptr<DesignerCanvas>    m_canvas;
    std::unique_ptr<PreviewManager>    m_preview;
};

}