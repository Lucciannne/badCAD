#include "part_editor.h"
#include "application.h"
#include "file_dialog.h"
#include "../core/document.h"
#include "../render/occ_viewer.h"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace badcad {

PartEditor::PartEditor(Application* app) 
    : m_app(app)
    , m_document(std::make_unique<Document>())
    , m_viewer(nullptr)  // Lazy initialization when viewport is first rendered
{
}

PartEditor::~PartEditor() {
}

void PartEditor::render() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Handle keyboard shortcuts
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        if (io.KeyShift) {
            // Ctrl+Shift+S = Save As
            saveFileAs();
        } else {
            // Ctrl+S = Save
            if (m_currentFilePath.empty()) {
                saveFileAs();
            } else {
                saveFile(m_currentFilePath);
            }
        }
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
        // Ctrl+O = Open
        openFile();
    }
    
    // Top toolbar
    if (ImGui::BeginMainMenuBar()) {
        // File menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Part", "Ctrl+N")) {
                newPart();
            }
            if (ImGui::MenuItem("New Assembly")) {
                // TODO: Implement assembly editor
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (m_currentFilePath.empty()) {
                    saveFileAs();
                } else {
                    saveFile(m_currentFilePath);
                }
            }
            if (ImGui::MenuItem("Save As...")) {
                saveFileAs();
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                openFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close", "Ctrl+W")) {
                m_app->setState(AppState::Home);
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
    
    float menuBarHeight = ImGui::GetFrameHeight();
    float toolbarHeight = 40.0f;
    float statusBarHeight = 25.0f;
    
    // Mode-specific toolbar
    ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, toolbarHeight));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    
    ImGui::Begin("Toolbar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);
    
    switch (m_mode) {
        case PartEditorMode::Model:
            renderModelToolbar();
            break;
        case PartEditorMode::Sketch:
            renderSketchToolbar();
            break;
        case PartEditorMode::Inspect:
            // Inspect mode has minimal toolbar
            ImGui::Text("Inspect Mode");
            break;
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
    
    float contentTop = menuBarHeight + toolbarHeight;
    float contentHeight = io.DisplaySize.y - contentTop - statusBarHeight;
    
    // Feature tree (left panel)
    float treeWidth = 250.0f;
    if (m_showFeatureTree) {
        ImGui::SetNextWindowPos(ImVec2(0, contentTop));
        ImGui::SetNextWindowSize(ImVec2(treeWidth, contentHeight));
        
        ImGui::Begin("Features", &m_showFeatureTree,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize);
        
        renderFeatureTree();
        
        ImGui::End();
    }
    
    // Properties panel (right panel)
    float propertiesWidth = 300.0f;
    if (m_showProperties) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - propertiesWidth, contentTop));
        ImGui::SetNextWindowSize(ImVec2(propertiesWidth, contentHeight));
        
        ImGui::Begin("Properties", &m_showProperties,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize);
        
        renderPropertiesPanel();
        
        ImGui::End();
    }
    
    // Constraints panel (right panel in sketch mode)
    if (m_mode == PartEditorMode::Sketch && m_showConstraints) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - propertiesWidth, contentTop));
        ImGui::SetNextWindowSize(ImVec2(propertiesWidth, contentHeight));
        
        ImGui::Begin("Constraints", &m_showConstraints,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize);
        
        renderConstraintsPanel();
        
        ImGui::End();
    }
    
    // Main 3D viewport
    float viewportLeft = m_showFeatureTree ? treeWidth : 0;
    float viewportWidth = io.DisplaySize.x - viewportLeft;
    if (m_showProperties || (m_mode == PartEditorMode::Sketch && m_showConstraints)) {
        viewportWidth -= propertiesWidth;
    }
    
    ImGui::SetNextWindowPos(ImVec2(viewportLeft, contentTop));
    ImGui::SetNextWindowSize(ImVec2(viewportWidth, contentHeight));
    
    ImGui::Begin("Viewport", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse);
    
    renderViewport();
    
    ImGui::End();
    
    // Status bar
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - statusBarHeight));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, statusBarHeight));
    
    ImGui::Begin("StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("Mode: %s", m_mode == PartEditorMode::Model ? "Model" : 
                            m_mode == PartEditorMode::Sketch ? "Sketch" : "Inspect");
    
    // Update and display status message
    if (m_statusMessageTime > 0.0f) {
        m_statusMessageTime -= io.DeltaTime;
        if (m_statusMessageTime < 0.0f) {
            m_statusMessage = "Ready";
        }
    }
    
    ImGui::SameLine(io.DisplaySize.x - 600);
    if (m_statusMessageTime > 0.0f && m_statusMessage != "Ready") {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", m_statusMessage.c_str());
    } else {
        ImGui::Text("%s", m_statusMessage.c_str());
    }
    
    ImGui::End();
    
    // Render save prompt modal if needed
    promptSaveChanges();
}

bool PartEditor::iconButton(const char* icon, const char* tooltip, const char* svgPath) {
    // TODO: Load SVG from svgPath if provided and exists
    // For now, use emoji fallback
    
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 8));
    bool clicked = ImGui::Button(icon, ImVec2(32, 32));
    ImGui::PopStyleVar();
    
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }
    
    return clicked;
}

void PartEditor::renderModelToolbar() {
    // Model mode tools
    ImGui::PushID("newsketch");
    if (iconButton("X", "New Sketch", "resources/icons/new_sketch.svg")) {
        std::string selectedPlane = m_document->getSelectedPlane();
        if (!selectedPlane.empty()) {
            // Create new sketch on selected plane
            m_document->createSketch(selectedPlane);
            m_hasUnsavedChanges = true;
            // Animate camera to be orthogonal to selected plane
            if (m_viewer) {
                m_viewer->animateToPlane(selectedPlane);
            }
            setMode(PartEditorMode::Sketch);
            m_statusMessage = "Sketch started on " + selectedPlane;
            m_statusMessageTime = 3.0f;
        } else {
            m_statusMessage = "Please select a plane first (click on XY, XZ, or YZ plane)";
            m_statusMessageTime = 5.0f;
        }
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("extrude");
    if (iconButton("X", "Extrude", "resources/icons/extrude.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("revolve");
    if (iconButton("X", "Revolve", "resources/icons/revolve.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("sweep");
    if (iconButton("X", "Sweep", "resources/icons/sweep.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("loft");
    if (iconButton("X", "Loft", "resources/icons/loft.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("boolean");
    if (iconButton("X", "Boolean", "resources/icons/boolean.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("fillet");
    if (iconButton("X", "Fillet", "resources/icons/fillet.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("chamfer");
    if (iconButton("X", "Chamfer", "resources/icons/chamfer.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("shell");
    if (iconButton("X", "Shell", "resources/icons/shell.svg")) {
    }
    ImGui::PopID();
}

void PartEditor::renderSketchToolbar() {
    // Drawing tools
    ImGui::PushID("point");
    bool isPointActive = (m_activeTool == SketchTool::Point);
    if (isPointActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
    }
    if (iconButton("X", "Point", "resources/icons/point.svg")) {
        m_activeTool = SketchTool::Point;
        m_lineStartPointIndex = -1;  // Reset line drawing state
        m_statusMessage = "Point tool active - Click to place points";
        m_statusMessageTime = 3.0f;
    }
    if (isPointActive) {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("line");
    bool isLineActive = (m_activeTool == SketchTool::Line);
    if (isLineActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
    }
    if (iconButton("X", "Line", "resources/icons/line.svg")) {
        m_activeTool = SketchTool::Line;
        m_lineStartPointIndex = -1;  // Reset line drawing state
        m_statusMessage = "Line tool active - Click to start line";
        m_statusMessageTime = 3.0f;
    }
    if (isLineActive) {
        ImGui::PopStyleColor();
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("arc");
    if (iconButton("X", "Arc", "resources/icons/arc.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("circle");
    if (iconButton("X", "Circle", "resources/icons/circle.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("rect");
    if (iconButton("X", "Rectangle", "resources/icons/rectangle.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("spline");
    if (iconButton("X", "Spline", "resources/icons/spline.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    // Constraint tools
    ImGui::PushID("coincident");
    if (iconButton("X", "Coincident", "resources/icons/coincident.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("tangent");
    if (iconButton("X", "Tangent", "resources/icons/tangent.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("perp");
    if (iconButton("X", "Perpendicular", "resources/icons/perpendicular.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("parallel");
    if (iconButton("X", "Parallel", "resources/icons/parallel.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    
    ImGui::PushID("dimension");
    if (iconButton("X", "Dimension", "resources/icons/dimension.svg")) {
    }
    ImGui::PopID();
    ImGui::SameLine(0, 4);
    ImGui::Separator();
    ImGui::SameLine(0, 4);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
    ImGui::PushID("exit");
    if (iconButton("X", "Exit Sketch", "resources/icons/exit_sketch.svg")) {
        // Clear editing flag on active sketch
        auto* activeSketch = m_document->getActiveSketch();
        if (activeSketch) {
            activeSketch->isEditing = false;
        }
        setMode(PartEditorMode::Model);
        m_activeTool = SketchTool::None;
        m_lineStartPointIndex = -1;  // Reset line drawing state
        m_statusMessage = "Exited sketch mode";
        m_statusMessageTime = 2.0f;
    }
    ImGui::PopID();
    ImGui::PopStyleColor();
}

void PartEditor::renderViewToolbar() {
    if (ImGui::Button("Fit")) {
        if (m_viewer) m_viewer->fitAll();
    }
    ImGui::SameLine();
    if (ImGui::Button("Front")) {
        if (m_viewer) m_viewer->setViewFront();
    }
    ImGui::SameLine();
    if (ImGui::Button("Top")) {
        if (m_viewer) m_viewer->setViewTop();
    }
    ImGui::SameLine();
    if (ImGui::Button("Right")) {
        if (m_viewer) m_viewer->setViewRight();
    }
    ImGui::SameLine();
    if (ImGui::Button("Iso")) {
        if (m_viewer) m_viewer->setViewIso();
    }
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    if (ImGui::Button("Wireframe")) {
    }
    ImGui::SameLine();
    if (ImGui::Button("Shaded")) {
    }
}

void PartEditor::renderFeatureTree() {
    ImGui::Text("Feature Tree");
    ImGui::Separator();
    
    // Construction Planes section
    if (ImGui::TreeNodeEx("Construction Planes", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto& planes = m_document->getPlanes();
        
        int planeIndex = 0;
        for (const auto& plane : planes) {
            ImGui::PushID(planeIndex++);
            
            // Eye icon button for visibility toggle
            const char* eyeIcon = plane.visible ? "👁" : "⚫";
            if (ImGui::SmallButton(eyeIcon)) {
                m_document->setPlaneVisibility(plane.name, !plane.visible);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip(plane.visible ? "Hide plane" : "Show plane");
            }
            
            ImGui::SameLine();
            
            // Selectable plane name with highlight if selected
            bool isSelected = m_document->isPlaneSelected(plane.name);
            if (ImGui::Selectable(plane.name.c_str(), isSelected, 0, ImVec2(0, 0))) {
                ImGuiIO& io = ImGui::GetIO();
                m_document->selectPlane(plane.name, io.KeyCtrl);
            }
            
            ImGui::PopID();
        }
        
        ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Sketches")) {
        const auto& sketches = m_document->getSketches();
        if (sketches.empty()) {
            ImGui::TextDisabled("(No sketches)");
        } else {
            for (const auto& sketch : sketches) {
                std::string displayName = sketch.name;
                if (sketch.isEditing) {
                    displayName += " (editing)";
                }
                displayName += " - " + std::to_string(sketch.points.size()) + " point(s)";
                
                // Make selectable with double-click to edit
                if (ImGui::Selectable(displayName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        // Double-click to edit sketch
                        // First, clear editing flag on all sketches
                        for (auto& s : m_document->getSketches()) {
                            const_cast<Document::Sketch&>(s).isEditing = false;
                        }
                        // Set this sketch as editing
                        const_cast<Document::Sketch&>(sketch).isEditing = true;
                        // Animate camera to the sketch's plane
                        if (m_viewer) {
                            m_viewer->animateToPlane(sketch.plane);
                        }
                        setMode(PartEditorMode::Sketch);
                        m_statusMessage = "Editing sketch: " + sketch.name;
                        m_statusMessageTime = 3.0f;
                    }
                }
            }
        }
        ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Features")) {
        ImGui::TextDisabled("(No features)");
        ImGui::TreePop();
    }
}

void PartEditor::renderViewport() {
    // Get the available size for the viewport
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // Get GLFW window and ensure its OpenGL context is current for all operations
    GLFWwindow* glfwWindow = m_app->getWindow();
    glfwMakeContextCurrent(glfwWindow);
    
    // Lazy initialize OpenCASCADE viewer on first render
    if (!m_viewer && viewportSize.x > 0 && viewportSize.y > 0) {
        // Debug: Check what texture ID ImGui is using for fonts
        ImGuiIO& io = ImGui::GetIO();
        ImTextureID fontTexId = io.Fonts->TexID;
        
        m_viewer = std::make_unique<OccViewer>();
        
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(glfwWindow);
        
        if (m_viewer->init(hwnd, (int)viewportSize.x, (int)viewportSize.y)) {
            m_viewer->setDocument(m_document.get());
        } else {
            std::cerr << "Failed to initialize OpenCASCADE viewer" << std::endl;
            m_viewer.reset();
        }
#endif
    }
    
    // Update viewer size if changed (with threshold to avoid constant recreation)
    static ImVec2 lastSize(0, 0);
    if (m_viewer) {
        // Update camera animation
        ImGuiIO& io = ImGui::GetIO();
        m_viewer->updateCameraAnimation(io.DeltaTime);
        
        // Only resize if the change is significant (more than 10 pixels)
        float deltaX = std::abs(viewportSize.x - lastSize.x);
        float deltaY = std::abs(viewportSize.y - lastSize.y);
        if (deltaX > 10 || deltaY > 10) {
            m_viewer->resize((int)viewportSize.x, (int)viewportSize.y);
            lastSize = viewportSize;
        }
    }
    
    // Render OpenCASCADE scene to FBO
    if (m_viewer) {
        // Update preview state for sketch tools
        if (m_mode == PartEditorMode::Sketch && m_activeTool == SketchTool::Line) {
            m_viewer->setPreviewState(m_lineStartPointIndex, m_hasMousePreview, 
                                     m_previewX, m_previewY, m_isSnapped, m_snappedX, m_snappedY);
        } else {
            m_viewer->setPreviewState(-1, false, 0, 0, false, 0, 0);
        }
        
        m_viewer->render();
        
        // Display the FBO texture in ImGui
        unsigned int texId = m_viewer->getTextureId();
        if (texId != 0) {
            static unsigned int lastTexId = 0;
            if (texId != lastTexId) {
                lastTexId = texId;
            }
            ImGui::Image((ImTextureID)(intptr_t)texId, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
            
            // Handle mouse interaction in viewport
            if (ImGui::IsItemHovered()) {
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 viewportPos = ImGui::GetItemRectMin();
                int localX = (int)(mousePos.x - viewportPos.x);
                int localY = (int)(mousePos.y - viewportPos.y);
                
                // Update hover state using ray casting
                std::string hoveredPlane = m_viewer->pickPlane(localX, localY, (int)viewportSize.x, (int)viewportSize.y);
                m_viewer->setHoveredPlane(hoveredPlane);
                
                // Middle mouse button - orbit
                if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !io.KeyShift) {
                    static bool rotationStarted = false;
                    if (!rotationStarted || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                        m_viewer->startRotation(localX, localY);
                        rotationStarted = true;
                    }
                    m_viewer->rotation(localX, localY);
                    
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
                        rotationStarted = false;
                    }
                }
                
                // Shift + Middle mouse button - pan
                if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && io.KeyShift) {
                    static bool panStarted = false;
                    if (!panStarted || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                        m_viewer->startPan(localX, localY);
                        panStarted = true;
                    }
                    m_viewer->pan(localX, localY);
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
                        panStarted = false;
                    }
                }
                
                // Mouse wheel - zoom
                float wheel = io.MouseWheel;
                if (wheel != 0.0f) {
                    m_viewer->zoom(wheel);
                }
                
                // Update mouse preview for sketch tools
                if (m_mode == PartEditorMode::Sketch && (m_activeTool == SketchTool::Line || m_activeTool == SketchTool::Point)) {
                    auto* activeSketch = m_document->getActiveSketch();
                    if (activeSketch && m_viewer) {
                        float x, y;
                        if (m_viewer->screenToPlanePoint((int)localX, (int)localY, 
                                                        (int)viewportSize.x, (int)viewportSize.y,
                                                        activeSketch->plane, x, y)) {
                            m_previewX = x;
                            m_previewY = y;
                            m_hasMousePreview = true;
                            
                            // Try snapping to existing points first
                            int snapPointIdx = -1;
                            const float snapRadius = 0.05f;  // 50mm snap radius
                            
                            m_isSnapped = m_document->snapToPoint(activeSketch, x, y, snapRadius, 
                                                                 m_snappedX, m_snappedY, snapPointIdx);
                            
                            if (m_isSnapped) {
                                m_snappedPointIndex = snapPointIdx;
                                m_snappedLineIndex = -1;
                            } else {
                                // If no point snap, try snapping to lines
                                int snapLineIdx = -1;
                                m_isSnapped = m_document->snapToLine(activeSketch, x, y, snapRadius, 
                                                                    m_snappedX, m_snappedY, snapLineIdx);
                                if (m_isSnapped) {
                                    m_snappedPointIndex = -1;
                                    m_snappedLineIndex = snapLineIdx;
                                } else {
                                    m_snappedPointIndex = -1;
                                    m_snappedLineIndex = -1;
                                }
                            }
                        } else {
                            m_hasMousePreview = false;
                        }
                    }
                } else {
                    m_hasMousePreview = false;
                }
                
                // Left mouse button - selection using ray casting
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    if (m_mode == PartEditorMode::Sketch && m_activeTool == SketchTool::Point) {
                        // In sketch mode with point tool: place a point
                        auto* activeSketch = m_document->getActiveSketch();
                        if (activeSketch && m_viewer) {
                            // Use snapped position if available, otherwise use raw mouse position
                            float x = m_isSnapped ? m_snappedX : m_previewX;
                            float y = m_isSnapped ? m_snappedY : m_previewY;
                            
                            m_document->addPointToSketch(activeSketch, x, y);
                            m_hasUnsavedChanges = true;
                        }
                    } else if (m_mode == PartEditorMode::Sketch && m_activeTool == SketchTool::Line) {
                        // In sketch mode with line tool: create line between two points
                        auto* activeSketch = m_document->getActiveSketch();
                        if (activeSketch && m_viewer) {
                            // Use snapped position if available
                            float x = m_isSnapped ? m_snappedX : m_previewX;
                            float y = m_isSnapped ? m_snappedY : m_previewY;
                            
                            // Add point (or reuse existing nearby point)
                            int pointIdx = m_document->addPointToSketch(activeSketch, x, y, false);
                            
                            if (m_lineStartPointIndex == -1) {
                                // First click - start the line
                                m_lineStartPointIndex = pointIdx;
                            } else {
                                // Check if this would close a shape
                                bool wouldClose = false;
                                if (m_snappedLineIndex != -1) {
                                    // Snapping to a line - check if that line connects back to start
                                    wouldClose = m_document->wouldCloseShapeOnLine(activeSketch, m_lineStartPointIndex, m_snappedLineIndex);
                                } else {
                                    // Snapping to a point or free click - check direct reachability
                                    wouldClose = m_document->wouldCloseShape(activeSketch, m_lineStartPointIndex, pointIdx);
                                }
                                
                                // Second click - complete the line
                                if (pointIdx != m_lineStartPointIndex) {
                                    // If we snapped to a line, split it at the snap point
                                    if (m_snappedLineIndex != -1) {
                                        m_document->splitLineAtPoint(activeSketch, m_snappedLineIndex, pointIdx);
                                    }
                                    
                                    m_document->addLineToSketch(activeSketch, m_lineStartPointIndex, pointIdx, false);
                                    m_hasUnsavedChanges = true;
                                }
                                
                                // If we closed a shape, reset line drawing; otherwise continue chain
                                if (wouldClose) {
                                    m_lineStartPointIndex = -1;
                                } else {
                                    m_lineStartPointIndex = pointIdx;
                                }
                            }
                        }
                    } else {
                        // Normal selection mode
                        std::string clickedPlane = m_viewer->pickPlane(localX, localY, (int)viewportSize.x, (int)viewportSize.y);
                        
                        if (!clickedPlane.empty()) {
                            // Clicked on a plane
                            m_document->selectPlane(clickedPlane, io.KeyCtrl);
                            m_statusMessage = "Selected " + clickedPlane + " (Click 'New Sketch' to start)";
                            m_statusMessageTime = 4.0f;
                        } else {
                            // Clicked on empty space, deselect all unless holding Ctrl
                            if (!io.KeyCtrl) {
                                m_document->deselectAll();
                            }
                        }
                    }
                }
            } else {
                // Mouse not over viewport, clear hover state
                m_viewer->setHoveredPlane("");
                m_hasMousePreview = false;
            }
            
            // Overlay info text
            ImDrawList* draw_list = ImGui::GetForegroundDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();
            p.y -= viewportSize.y;
            
            const char* info = "3D Viewport - OpenGL rendering active";
            draw_list->AddText(ImVec2(p.x + 10, p.y + viewportSize.y - 30),
                             IM_COL32(180, 180, 180, 200), info);
        }
    } else {
        // Show placeholder if viewer not initialized yet
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        
        // Dark background
        draw_list->AddRectFilled(p, ImVec2(p.x + viewportSize.x, p.y + viewportSize.y),
                                 IM_COL32(25, 25, 30, 255));
        
        const char* text = "Initializing 3D Viewport...";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        draw_list->AddText(ImVec2(p.x + (viewportSize.x - textSize.x) * 0.5f,
                                  p.y + (viewportSize.y - textSize.y) * 0.5f),
                           IM_COL32(150, 150, 160, 255), text);
    }
}

void PartEditor::renderPropertiesPanel() {
    ImGui::Text("Properties");
    ImGui::Separator();
    ImGui::TextDisabled("No selection");
}

void PartEditor::renderConstraintsPanel() {
    ImGui::Text("Constraints");
    ImGui::Separator();
    ImGui::TextDisabled("No constraints");
}

// File operations
std::string PartEditor::openFileDialog(bool save) {
#ifdef _WIN32
    HWND hwnd = nullptr;
    if (m_app) {
        GLFWwindow* window = m_app->getWindow();
        if (window) {
            hwnd = glfwGetWin32Window(window);
        }
    }
    return badcad::openFileDialog(save, hwnd);
#else
    return "";
#endif
}

void PartEditor::saveFile(const std::string& path) {
    if (path.empty()) {
        return;
    }
    
    std::string data = m_document->serialize();
    std::ofstream file(path);
    if (file.is_open()) {
        file << data;
        file.close();
        m_currentFilePath = path;
        m_hasUnsavedChanges = false;
        
        // Update window title with filename
        size_t lastSlash = path.find_last_of("/\\");
        std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
        m_app->setWindowTitle("badCAD - " + filename);
        
        m_statusMessage = "File saved: " + filename;
        m_statusMessageTime = 3.0f;
        
    } else {
        std::cerr << "Failed to save file: " << path << std::endl;
        m_statusMessage = "ERROR: Failed to save file";
        m_statusMessageTime = 5.0f;
    }
}

void PartEditor::saveFileAs() {
    std::string path = openFileDialog(true);
    if (!path.empty()) {
        saveFile(path);
    }
}

void PartEditor::openFile() {
    std::string path = openFileDialog(false);
    if (!path.empty()) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::string data((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
            file.close();
            
            m_document->deserialize(data);
            m_currentFilePath = path;
            m_hasUnsavedChanges = false;
            
            // Refresh viewer
            if (m_viewer) {
                m_viewer->setDocument(m_document.get());
                m_viewer->updateFromDocument();
            }
            
            // Update window title with filename
            size_t lastSlash = path.find_last_of("/\\");
            std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
            m_app->setWindowTitle("badCAD - " + filename);
            
            m_statusMessage = "File loaded: " + filename;
            m_statusMessageTime = 3.0f;
            
        } else {
            std::cerr << "Failed to open file: " << path << std::endl;
            m_statusMessage = "ERROR: Failed to open file";
            m_statusMessageTime = 5.0f;
        }
    }
}

void PartEditor::newPart() {
    // Check for unsaved changes
    if (m_hasUnsavedChanges) {
        m_showSavePrompt = true;
    } else {
        resetDocument();
    }
}

bool PartEditor::promptSaveChanges() {
    // This will be rendered in the main render loop
    if (!m_showSavePrompt) {
        return false;
    }
    
    bool result = false;
    ImGui::OpenPopup("Unsaved Changes");
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Do you want to save your changes?");
        ImGui::Separator();
        
        if (ImGui::Button("Save", ImVec2(120, 0))) {
            if (m_currentFilePath.empty()) {
                saveFileAs();
            } else {
                saveFile(m_currentFilePath);
            }
            if (!m_hasUnsavedChanges) {  // Save was successful
                resetDocument();
                m_showSavePrompt = false;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't Save", ImVec2(120, 0))) {
            resetDocument();
            m_showSavePrompt = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showSavePrompt = false;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
        result = true;
    }
    
    return result;
}

void PartEditor::resetDocument() {
    // Create a fresh document
    m_document = std::make_unique<Document>();
    
    // Reset state
    m_currentFilePath.clear();
    m_hasUnsavedChanges = false;
    m_mode = PartEditorMode::Model;
    m_activeTool = SketchTool::None;
    m_lineStartPointIndex = -1;  // Reset line drawing state
    
    // Refresh viewer with new document
    if (m_viewer) {
        m_viewer->setDocument(m_document.get());
    }
    
    // Update window title
    m_app->setWindowTitle("badCAD - Unsaved Part");
    
}

} // namespace badcad
