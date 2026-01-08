#include "part_editor.h"
#include "application.h"
#include "../core/document.h"
#include "../render/occ_viewer.h"
#include <imgui.h>
#include <iostream>
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
    std::cout << "Part Editor created with document:\n" << m_document->serialize() << std::endl;
}

PartEditor::~PartEditor() {
}

void PartEditor::render() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Top toolbar
    if (ImGui::BeginMainMenuBar()) {
        // File menu
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                std::cout << "Save clicked" << std::endl;
            }
            if (ImGui::MenuItem("Save As...")) {
                std::cout << "Save As clicked" << std::endl;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close", "Ctrl+W")) {
                std::cout << "Close clicked" << std::endl;
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
    ImGui::SameLine(io.DisplaySize.x - 200);
    ImGui::Text("Ready");
    
    ImGui::End();
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
    if (iconButton("✏️", "New Sketch", "resources/icons/new_sketch.svg")) {
        std::cout << "New Sketch clicked" << std::endl;
        setMode(PartEditorMode::Sketch);
    }
    ImGui::SameLine();
    
    if (iconButton("⬆️", "Extrude", "resources/icons/extrude.svg")) {
        std::cout << "Extrude clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("🔄", "Revolve", "resources/icons/revolve.svg")) {
        std::cout << "Revolve clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("🔀", "Sweep", "resources/icons/sweep.svg")) {
        std::cout << "Sweep clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("📐", "Loft", "resources/icons/loft.svg")) {
        std::cout << "Loft clicked" << std::endl;
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    if (iconButton("➕", "Boolean Union", "resources/icons/bool_union.svg")) {
        std::cout << "Boolean Union clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("➖", "Boolean Cut", "resources/icons/bool_cut.svg")) {
        std::cout << "Boolean Cut clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("✂️", "Boolean Intersect", "resources/icons/bool_intersect.svg")) {
        std::cout << "Boolean Intersect clicked" << std::endl;
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    if (iconButton("🟢", "Fillet", "resources/icons/fillet.svg")) {
        std::cout << "Fillet clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("📐", "Chamfer", "resources/icons/chamfer.svg")) {
        std::cout << "Chamfer clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("🔲", "Shell", "resources/icons/shell.svg")) {
        std::cout << "Shell clicked" << std::endl;
    }
}

void PartEditor::renderSketchToolbar() {
    // Drawing tools
    if (iconButton("📏", "Line", "resources/icons/line.svg")) {
        std::cout << "Line tool clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("⌒", "Arc", "resources/icons/arc.svg")) {
        std::cout << "Arc tool clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("⭕", "Circle", "resources/icons/circle.svg")) {
        std::cout << "Circle tool clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("▭", "Rectangle", "resources/icons/rectangle.svg")) {
        std::cout << "Rectangle tool clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("〜", "Spline", "resources/icons/spline.svg")) {
        std::cout << "Spline tool clicked" << std::endl;
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Constraint tools
    if (iconButton("⊙", "Coincident", "resources/icons/coincident.svg")) {
        std::cout << "Coincident constraint clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("⟂", "Tangent", "resources/icons/tangent.svg")) {
        std::cout << "Tangent constraint clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("⊥", "Perpendicular", "resources/icons/perpendicular.svg")) {
        std::cout << "Perpendicular constraint clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("∥", "Parallel", "resources/icons/parallel.svg")) {
        std::cout << "Parallel constraint clicked" << std::endl;
    }
    ImGui::SameLine();
    
    if (iconButton("📐", "Dimension", "resources/icons/dimension.svg")) {
        std::cout << "Dimension tool clicked" << std::endl;
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Exit sketch with different styling
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
    if (iconButton("✓", "Exit Sketch", "resources/icons/exit_sketch.svg")) {
        std::cout << "Exit Sketch clicked" << std::endl;
        setMode(PartEditorMode::Model);
    }
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
        std::cout << "Wireframe mode clicked" << std::endl;
    }
    ImGui::SameLine();
    if (ImGui::Button("Shaded")) {
        std::cout << "Shaded mode clicked" << std::endl;
    }
}

void PartEditor::renderFeatureTree() {
    ImGui::Text("Feature Tree");
    ImGui::Separator();
    
    // Construction Planes section
    if (ImGui::TreeNodeEx("Construction Planes", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto& planes = m_document->getPlanes();
        
        for (const auto& plane : planes) {
            ImGui::PushID(plane.name.c_str());
            
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
        ImGui::TextDisabled("(No sketches)");
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
        std::cout << "ImGui font atlas texture ID: " << (intptr_t)fontTexId << std::endl;
        
        m_viewer = std::make_unique<OccViewer>();
        
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(glfwWindow);
        
        if (m_viewer->init(hwnd, (int)viewportSize.x, (int)viewportSize.y)) {
            m_viewer->setDocument(m_document.get());
            std::cout << "OpenCASCADE viewer initialized in viewport" << std::endl;
        } else {
            std::cerr << "Failed to initialize OpenCASCADE viewer" << std::endl;
            m_viewer.reset();
        }
#endif
    }
    
    // Update viewer size if changed (with threshold to avoid constant recreation)
    static ImVec2 lastSize(0, 0);
    if (m_viewer) {
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
        m_viewer->render();
        
        // Display the FBO texture in ImGui
        unsigned int texId = m_viewer->getTextureId();
        if (texId != 0) {
            static unsigned int lastTexId = 0;
            if (texId != lastTexId) {
                std::cout << "Displaying texture ID: " << texId << std::endl;
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
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) && !io.KeyShift) {
                    static bool rotationStarted = false;
                    if (!rotationStarted) {
                        m_viewer->startRotation(localX, localY);
                        rotationStarted = true;
                    } else {
                        m_viewer->rotation(localX, localY);
                    }
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
                        rotationStarted = false;
                    }
                }
                
                // Shift + Middle mouse button - pan
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) && io.KeyShift) {
                    static bool panStarted = false;
                    if (!panStarted) {
                        m_viewer->startPan(localX, localY);
                        panStarted = true;
                    } else {
                        m_viewer->pan(localX, localY);
                    }
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
                        panStarted = false;
                    }
                }
                
                // Mouse wheel - zoom
                float wheel = io.MouseWheel;
                if (wheel != 0.0f) {
                    m_viewer->zoom(wheel);
                }
                
                // Left mouse button - selection using ray casting
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    std::string clickedPlane = m_viewer->pickPlane(localX, localY, (int)viewportSize.x, (int)viewportSize.y);
                    
                    if (!clickedPlane.empty()) {
                        // Clicked on a plane
                        m_document->selectPlane(clickedPlane, io.KeyCtrl);
                    } else {
                        // Clicked on empty space, deselect all unless holding Ctrl
                        if (!io.KeyCtrl) {
                            m_document->deselectAll();
                        }
                    }
                }
            } else {
                // Mouse not over viewport, clear hover state
                m_viewer->setHoveredPlane("");
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

} // namespace badcad
