#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <OpenGl_GraphicDriver.hxx>

#ifdef _WIN32
#include <windows.h>
#endif

namespace badcad {

class Document;

class OccViewer {
public:
    OccViewer();
    ~OccViewer();
    
    bool init(void* windowHandle, int width, int height);
    void resize(int width, int height);
    void render();
    
    // Get OpenGL texture ID for ImGui
    unsigned int getTextureId() const { return m_fboTexture; }
    
    // Document integration
    void setDocument(Document* doc);
    void updateFromDocument();
    
    // Camera controls
    void fitAll();
    void setViewFront();
    void setViewTop();
    void setViewRight();
    void setViewIso();
    
    // Interaction
    void startRotation(int x, int y);
    void rotation(int x, int y);
    void startPan(int x, int y);
    void pan(int x, int y);
    void zoom(float factor);
    
    // Picking
    std::string pickPlane(int mouseX, int mouseY, int viewportWidth, int viewportHeight);
    void setHoveredPlane(const std::string& planeName);
    
private:
    void createDefaultPlanes();
    void updatePlaneVisibility();
    void createFramebuffer(int width, int height);
    void deleteFramebuffer();
    
    Handle(V3d_Viewer) m_viewer;
    Handle(V3d_View) m_view;
    Handle(AIS_InteractiveContext) m_context;
    
    Document* m_document = nullptr;
    
    // OpenGL FBO for rendering
    unsigned int m_fbo = 0;
    unsigned int m_fboTexture = 0;
    unsigned int m_fboDepth = 0;
    int m_fboWidth = 0;
    int m_fboHeight = 0;
    
    // Camera state
    float m_cameraDistance = 2.0f;
    float m_cameraRotX = 30.0f;  // degrees
    float m_cameraRotY = 0.0f;
    float m_cameraRotZ = -45.0f;
    float m_cameraPanX = 0.0f;
    float m_cameraPanY = 0.0f;
    
    // Interaction state
    int m_lastX = 0;
    int m_lastY = 0;
    
    // Picking state
    std::string m_hoveredPlane;
};

} // namespace badcad
