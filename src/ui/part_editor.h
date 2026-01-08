#pragma once

#include <memory>

namespace badcad {

// Forward declarations
class Document;
class OccViewer;

enum class PartEditorMode {
    Model,
    Sketch,
    Inspect
};

class Application;

class PartEditor {
public:
    PartEditor(Application* app);
    ~PartEditor();
    
    void render();
    
    PartEditorMode getMode() const { return m_mode; }
    void setMode(PartEditorMode mode) { m_mode = mode; }
    
private:
    void renderModelToolbar();
    void renderSketchToolbar();
    void renderViewToolbar();
    
    // Helper for icon buttons
    bool iconButton(const char* icon, const char* tooltip, const char* svgPath = nullptr);
    void renderFeatureTree();
    void renderViewport();
    void renderPropertiesPanel();
    void renderConstraintsPanel();
    
    Application* m_app;
    PartEditorMode m_mode = PartEditorMode::Model;
    bool m_showFeatureTree = true;
    bool m_showProperties = false;
    bool m_showConstraints = false;
    
    std::unique_ptr<Document> m_document;
    std::unique_ptr<OccViewer> m_viewer;
};

} // namespace badcad
