#pragma once

#include <memory>
#include <string>

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
    
    // Public file operations (accessible from home screen)
    void openFile();
    
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
    
    // File operations
    void newPart();
    void saveFile(const std::string& path);
    void saveFileAs();
    std::string openFileDialog(bool save);
    bool promptSaveChanges();
    void resetDocument();
    
    Application* m_app;
    PartEditorMode m_mode = PartEditorMode::Model;
    bool m_showFeatureTree = true;
    bool m_showProperties = false;
    bool m_showConstraints = false;
    
    std::string m_currentFilePath;
    bool m_hasUnsavedChanges = false;
    bool m_showSavePrompt = false;
    
    // Sketch tool selection
    enum class SketchTool {
        None,
        Point,
        Line,
        Circle,
        Arc,
        Rectangle,
        Spline
    };
    SketchTool m_activeTool = SketchTool::None;
    
    // Line drawing state
    int m_lineStartPointIndex = -1;  // -1 means no line in progress
    
    // Mouse preview for sketch tools
    bool m_hasMousePreview = false;
    float m_previewX = 0.0f;
    float m_previewY = 0.0f;
    float m_snappedX = 0.0f;
    float m_snappedY = 0.0f;
    bool m_isSnapped = false;
    int m_snappedPointIndex = -1;   // If snapped to point
    int m_snappedLineIndex = -1;    // If snapped to line
    
    // Status message
    std::string m_statusMessage = "Ready";
    float m_statusMessageTime = 0.0f;
    
    std::unique_ptr<Document> m_document;
    std::unique_ptr<OccViewer> m_viewer;
};

} // namespace badcad
