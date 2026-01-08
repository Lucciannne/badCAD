#define _USE_MATH_DEFINES
#include <cmath>

#include "occ_viewer.h"
#include "../core/document.h"
#include <iostream>

#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Plane.hxx>
#include <Geom_Plane.hxx>
#include <gp_Pln.hxx>
#include <gp_Ax3.hxx>
#include <Quantity_Color.hxx>
#include <Graphic3d_MaterialAspect.hxx>

// OpenGL and GLFW must be included before platform-specific headers
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <WNT_Window.hxx>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <GL/glext.h>  // For OpenGL framebuffer extensions
#else
#include <Xw_Window.hxx>
#endif

namespace badcad {

// OpenGL FBO function pointers (loaded at runtime)
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = nullptr;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = nullptr;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = nullptr;
static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = nullptr;
static PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = nullptr;
static PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = nullptr;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = nullptr;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = nullptr;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = nullptr;
static PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = nullptr;

static bool loadGLExtensions() {
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glfwGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glfwGetProcAddress("glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glfwGetProcAddress("glFramebufferTexture2D");
    glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glfwGetProcAddress("glGenRenderbuffers");
    glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glfwGetProcAddress("glBindRenderbuffer");
    glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glfwGetProcAddress("glRenderbufferStorage");
    glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glfwGetProcAddress("glFramebufferRenderbuffer");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glfwGetProcAddress("glCheckFramebufferStatus");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glfwGetProcAddress("glDeleteFramebuffers");
    glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)glfwGetProcAddress("glDeleteRenderbuffers");
    
    return glGenFramebuffers && glBindFramebuffer && glFramebufferTexture2D &&
           glGenRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glFramebufferRenderbuffer && glCheckFramebufferStatus &&
           glDeleteFramebuffers && glDeleteRenderbuffers;
}

OccViewer::OccViewer() {
}

OccViewer::~OccViewer() {
    deleteFramebuffer();
}

bool OccViewer::init(void* windowHandle, int width, int height) {
    try {
        std::cout << "OccViewer::init() called with size " << width << "x" << height << std::endl;
        
        // Load OpenGL extensions
        std::cout << "Loading OpenGL FBO extensions..." << std::endl;
        if (!loadGLExtensions()) {
            std::cerr << "Failed to load OpenGL FBO extensions" << std::endl;
            return false;
        }
        std::cout << "OpenGL FBO extensions loaded successfully" << std::endl;
        
        // For now, skip OpenCASCADE window setup to avoid FBO conflicts
        // We'll render with our own OpenGL code until we solve the FBO coordination issue
        std::cout << "Skipping OpenCASCADE window setup (will use direct OpenGL rendering)" << std::endl;
        
        // TODO: Set up OpenCASCADE with proper offscreen rendering
        // For now, we'll just use our FBO for simple test rendering
        
        // Create framebuffer for rendering
        std::cout << "Creating framebuffer..." << std::endl;
        createFramebuffer(width, height);
        std::cout << "Framebuffer created" << std::endl;
        
        // Set initial camera position (isometric)
        std::cout << "Setting isometric view..." << std::endl;
        setViewIso();
        
        std::cout << "OpenCASCADE viewer initialized successfully" << std::endl;
        return true;
        
    } catch (Standard_Failure const& e) {
        std::cerr << "OpenCASCADE initialization failed: " << e.GetMessageString() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "OpenCASCADE initialization failed with unknown exception" << std::endl;
        return false;
    }
}

void OccViewer::resize(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    
    // Recreate framebuffer with new size
    if (width != m_fboWidth || height != m_fboHeight) {
        createFramebuffer(width, height);
    }
    
    // Don't call m_view->MustBeResized() - it creates OpenCASCADE FBOs that conflict with ours
}

void OccViewer::render() {
    if (m_fbo == 0 || m_fboTexture == 0) {
        std::cerr << "render() called with invalid FBO or texture" << std::endl;
        return;
    }
    
    try {
        // Bind our FBO
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glViewport(0, 0, m_fboWidth, m_fboHeight);
        
        // Clear to dark CAD background
        glClearColor(0.15f, 0.15f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw a simple test pattern to show FBO rendering is working
        // Set up projection with correct aspect ratio
        float aspect = (float)m_fboWidth / (float)m_fboHeight;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (aspect > 1.0f) {
            // Wider than tall - increase near/far clipping planes to prevent geometry disappearing
            glOrtho(-aspect, aspect, -1, 1, -100, 100);
        } else {
            // Taller than wide
            glOrtho(-1, 1, -1.0f/aspect, 1.0f/aspect, -100, 100);
        }
        
        // Apply camera transformations
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // Apply zoom (distance from origin)
        glScalef(1.0f / m_cameraDistance, 1.0f / m_cameraDistance, 1.0f / m_cameraDistance);
        
        // Apply pan
        glTranslatef(m_cameraPanX, m_cameraPanY, 0.0f);
        
        // Apply rotations (reverse order for proper camera behavior)
        glRotatef(m_cameraRotX, 1.0f, 0.0f, 0.0f);
        glRotatef(m_cameraRotY, 0.0f, 1.0f, 0.0f);
        glRotatef(m_cameraRotZ, 0.0f, 0.0f, 1.0f);
        
        // Draw construction plane axes (only for visible planes)
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        
        // X and Y axes (for XY plane)
        if (m_document && m_document->isPlaneVisible("planexy")) {
            // X axis (RED) - horizontal
            glColor3f(0.8f, 0.2f, 0.2f);
            glVertex3f(-0.9f, 0.0f, 0.0f);
            glVertex3f(0.9f, 0.0f, 0.0f);
            // X axis arrow
            glVertex3f(0.9f, 0.0f, 0.0f);
            glVertex3f(0.8f, 0.05f, 0.0f);
            glVertex3f(0.9f, 0.0f, 0.0f);
            glVertex3f(0.8f, -0.05f, 0.0f);
            
            // Y axis (GREEN) - vertical
            glColor3f(0.2f, 0.8f, 0.2f);
            glVertex3f(0.0f, -0.9f, 0.0f);
            glVertex3f(0.0f, 0.9f, 0.0f);
            // Y axis arrow
            glVertex3f(0.0f, 0.9f, 0.0f);
            glVertex3f(0.05f, 0.8f, 0.0f);
            glVertex3f(0.0f, 0.9f, 0.0f);
            glVertex3f(-0.05f, 0.8f, 0.0f);
        }
        
        // X and Z axes (for XZ plane)
        if (m_document && m_document->isPlaneVisible("planexz")) {
            // X axis (RED) - horizontal
            glColor3f(0.8f, 0.2f, 0.2f);
            glVertex3f(-0.9f, 0.0f, 0.0f);
            glVertex3f(0.9f, 0.0f, 0.0f);
            // X axis arrow
            glVertex3f(0.9f, 0.0f, 0.0f);
            glVertex3f(0.8f, 0.0f, 0.05f);
            glVertex3f(0.9f, 0.0f, 0.0f);
            glVertex3f(0.8f, 0.0f, -0.05f);
            
            // Z axis (BLUE) - depth
            glColor3f(0.3f, 0.5f, 1.0f);
            glVertex3f(0.0f, 0.0f, -0.9f);
            glVertex3f(0.0f, 0.0f, 0.9f);
            // Z axis arrow
            glVertex3f(0.0f, 0.0f, 0.9f);
            glVertex3f(0.05f, 0.0f, 0.8f);
            glVertex3f(0.0f, 0.0f, 0.9f);
            glVertex3f(-0.05f, 0.0f, 0.8f);
        }
        
        // Y and Z axes (for YZ plane)
        if (m_document && m_document->isPlaneVisible("planeyz")) {
            // Y axis (GREEN) - vertical
            glColor3f(0.2f, 0.8f, 0.2f);
            glVertex3f(0.0f, -0.9f, 0.0f);
            glVertex3f(0.0f, 0.9f, 0.0f);
            // Y axis arrow
            glVertex3f(0.0f, 0.9f, 0.0f);
            glVertex3f(0.0f, 0.8f, 0.05f);
            glVertex3f(0.0f, 0.9f, 0.0f);
            glVertex3f(0.0f, 0.8f, -0.05f);
            
            // Z axis (BLUE) - depth
            glColor3f(0.3f, 0.5f, 1.0f);
            glVertex3f(0.0f, 0.0f, -0.9f);
            glVertex3f(0.0f, 0.0f, 0.9f);
            // Z axis arrow
            glVertex3f(0.0f, 0.0f, 0.9f);
            glVertex3f(0.0f, 0.05f, 0.8f);
            glVertex3f(0.0f, 0.0f, 0.9f);
            glVertex3f(0.0f, -0.05f, 0.8f);
        }
        
        glEnd();
        
        // Draw three construction planes as semi-transparent quads
        // Using ImGui button blue color for consistency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float planeSize = 0.7f;
        
        // XY Plane (Z=0)
        if (m_document && m_document->isPlaneVisible("planexy")) {
            bool isSelected = m_document->isPlaneSelected("planexy");
            bool isHovered = (m_hoveredPlane == "planexy");
            
            glBegin(GL_QUADS);
            if (isSelected) {
                glColor4f(1.0f, 0.6f, 0.0f, 0.25f);  // Brighter orange for selection
            } else if (isHovered) {
                glColor4f(0.8f, 0.45f, 0.0f, 0.20f);  // Darker orange for hover
            } else {
                glColor4f(0.26f, 0.59f, 0.98f, 0.15f);  // Semi-transparent ImGui blue
            }
            glVertex3f(-planeSize, -planeSize, 0.0f);
            glVertex3f(planeSize, -planeSize, 0.0f);
            glVertex3f(planeSize, planeSize, 0.0f);
            glVertex3f(-planeSize, planeSize, 0.0f);
            glEnd();
            
            // Draw border
            glLineWidth((isSelected || isHovered) ? 4.0f : 2.0f);
            glBegin(GL_LINE_LOOP);
            if (isSelected) {
                glColor4f(1.0f, 0.6f, 0.0f, 1.0f);  // Solid orange border
            } else if (isHovered) {
                glColor4f(0.8f, 0.45f, 0.0f, 0.8f);  // Darker orange border
            } else {
                glColor4f(0.26f, 0.59f, 0.98f, 0.6f);
            }
            glVertex3f(-planeSize, -planeSize, 0.0f);
            glVertex3f(planeSize, -planeSize, 0.0f);
            glVertex3f(planeSize, planeSize, 0.0f);
            glVertex3f(-planeSize, planeSize, 0.0f);
            glEnd();
        }
        
        // XZ Plane (Y=0)
        if (m_document && m_document->isPlaneVisible("planexz")) {
            bool isSelected = m_document->isPlaneSelected("planexz");
            bool isHovered = (m_hoveredPlane == "planexz");
            
            glBegin(GL_QUADS);
            if (isSelected) {
                glColor4f(1.0f, 0.6f, 0.0f, 0.25f);  // Brighter orange for selection
            } else if (isHovered) {
                glColor4f(0.8f, 0.45f, 0.0f, 0.20f);  // Darker orange for hover
            } else {
                glColor4f(0.26f, 0.59f, 0.98f, 0.15f);  // Semi-transparent ImGui blue
            }
            glVertex3f(-planeSize, 0.0f, -planeSize);
            glVertex3f(planeSize, 0.0f, -planeSize);
            glVertex3f(planeSize, 0.0f, planeSize);
            glVertex3f(-planeSize, 0.0f, planeSize);
            glEnd();
            
            // Draw border
            glLineWidth((isSelected || isHovered) ? 4.0f : 2.0f);
            glBegin(GL_LINE_LOOP);
            if (isSelected) {
                glColor4f(1.0f, 0.6f, 0.0f, 1.0f);  // Solid orange border
            } else if (isHovered) {
                glColor4f(0.8f, 0.45f, 0.0f, 0.8f);  // Darker orange border
            } else {
                glColor4f(0.26f, 0.59f, 0.98f, 0.6f);
            }
            glVertex3f(-planeSize, 0.0f, -planeSize);
            glVertex3f(planeSize, 0.0f, -planeSize);
            glVertex3f(planeSize, 0.0f, planeSize);
            glVertex3f(-planeSize, 0.0f, planeSize);
            glEnd();
        }
        
        // YZ Plane (X=0)
        if (m_document && m_document->isPlaneVisible("planeyz")) {
            bool isSelected = m_document->isPlaneSelected("planeyz");
            bool isHovered = (m_hoveredPlane == "planeyz");
            
            glBegin(GL_QUADS);
            if (isSelected) {
                glColor4f(1.0f, 0.6f, 0.0f, 0.25f);  // Brighter orange for selection
            } else if (isHovered) {
                glColor4f(0.8f, 0.45f, 0.0f, 0.20f);  // Darker orange for hover
            } else {
                glColor4f(0.26f, 0.59f, 0.98f, 0.15f);  // Semi-transparent ImGui blue
            }
            glVertex3f(0.0f, -planeSize, -planeSize);
            glVertex3f(0.0f, planeSize, -planeSize);
            glVertex3f(0.0f, planeSize, planeSize);
            glVertex3f(0.0f, -planeSize, planeSize);
            glEnd();
            
            // Draw border
            glLineWidth((isSelected || isHovered) ? 4.0f : 2.0f);
            glBegin(GL_LINE_LOOP);
            if (isSelected) {
                glColor4f(1.0f, 0.6f, 0.0f, 1.0f);  // Solid orange border
            } else if (isHovered) {
                glColor4f(0.8f, 0.45f, 0.0f, 0.8f);  // Darker orange border
            } else {
                glColor4f(0.26f, 0.59f, 0.98f, 0.6f);
            }
            glVertex3f(0.0f, -planeSize, -planeSize);
            glVertex3f(0.0f, planeSize, -planeSize);
            glVertex3f(0.0f, planeSize, planeSize);
            glVertex3f(0.0f, -planeSize, planeSize);
            glEnd();
        }
        
        glDisable(GL_BLEND);
        
        // TODO: Once we solve the window mapping issue, we can call m_view->Redraw() here
        
        // IMPORTANT: Make sure our texture is properly flushed
        glFlush();
        
        // Unbind FBO and texture to prevent interference with ImGui
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    } catch (Standard_Failure const& e) {
        std::cerr << "Error during render: " << e.GetMessageString() << std::endl;
    } catch (...) {
        std::cerr << "Error during render" << std::endl;
    }
}

void OccViewer::createFramebuffer(int width, int height) {
    if (width <= 0 || height <= 0) {
        return;
    }
    
    // If we already have an FBO of the right size, don't recreate
    if (m_fbo != 0 && m_fboWidth == width && m_fboHeight == height) {
        return;
    }
    
    bool isResize = (m_fbo != 0);
    
    if (isResize) {
        // Resize existing textures instead of deleting/recreating
        std::cout << "Resizing FBO from " << m_fboWidth << "x" << m_fboHeight 
                  << " to " << width << "x" << height << std::endl;
        
        m_fboWidth = width;
        m_fboHeight = height;
        
        // Resize color texture
        glBindTexture(GL_TEXTURE_2D, m_fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        
        // Resize depth renderbuffer
        glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        
        // Unbind
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        std::cout << "FBO resized: " << width << "x" << height 
                  << " FBO=" << m_fbo << " Texture=" << m_fboTexture << std::endl;
    } else {
        // Initial creation
        m_fboWidth = width;
        m_fboHeight = height;
        
        // Create framebuffer
        glGenFramebuffers(1, &m_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        
        // Create color texture
        glGenTextures(1, &m_fboTexture);
        glBindTexture(GL_TEXTURE_2D, m_fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fboTexture, 0);
        
        // Create depth renderbuffer
        glGenRenderbuffers(1, &m_fboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, m_fboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_fboDepth);
        
        // Check framebuffer status
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete: " << status << std::endl;
            deleteFramebuffer();
        } else {
            std::cout << "Framebuffer created: " << width << "x" << height 
                      << " FBO=" << m_fbo << " Texture=" << m_fboTexture << std::endl;
        }
        
        // Unbind framebuffer and textures
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}

void OccViewer::deleteFramebuffer() {
    if (m_fbo) {
        std::cout << "Deleting FBO " << m_fbo << " with texture " << m_fboTexture << std::endl;
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    if (m_fboTexture) {
        glDeleteTextures(1, &m_fboTexture);
        m_fboTexture = 0;
    }
    if (m_fboDepth) {
        glDeleteRenderbuffers(1, &m_fboDepth);
        m_fboDepth = 0;
    }
    m_fboWidth = 0;
    m_fboHeight = 0;
}

void OccViewer::setDocument(Document* doc) {
    m_document = doc;
    updateFromDocument();
}

void OccViewer::updateFromDocument() {
    if (!m_document) {
        return;
    }
    
    std::cout << "Document state:\n" << m_document->serialize() << std::endl;
    
    // Note: OpenCASCADE's AIS_InteractiveContext->Display() requires a properly mapped window
    // For offscreen FBO rendering, we need to use a different approach
    // For now, we'll demonstrate the infrastructure is working without crashing
    
    std::cout << "OpenCASCADE viewer ready (plane rendering requires window mapping)" << std::endl;
}

void OccViewer::createDefaultPlanes() {
    if (m_context.IsNull() || !m_document) {
        std::cout << "createDefaultPlanes: context or document is null" << std::endl;
        return;
    }
    
    std::cout << "Creating construction planes..." << std::endl;
    
    try {
        const double planeSize = 100.0;
        
        // XY Plane (Z=0, blue)
        if (m_document->isPlaneVisible("planexy")) {
            std::cout << "Creating XY plane..." << std::endl;
            try {
                gp_Pln xyPlane(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
                Handle(Geom_Plane) geomXY = new Geom_Plane(xyPlane);
                Handle(AIS_Plane) aisXY = new AIS_Plane(geomXY);
                aisXY->SetSize(planeSize, planeSize);
                aisXY->SetColor(Quantity_NOC_BLUE1);
                aisXY->SetTransparency(0.8);
                std::cout << "Displaying XY plane..." << std::endl;
                m_context->Display(aisXY, Standard_False);
                std::cout << "XY plane created successfully" << std::endl;
            } catch (Standard_Failure const& e) {
                std::cerr << "Error creating XY plane: " << e.GetMessageString() << std::endl;
            }
        }
        
        // XZ Plane (Y=0, green)
        if (m_document->isPlaneVisible("planexz")) {
            std::cout << "Creating XZ plane..." << std::endl;
            gp_Pln xzPlane(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
            Handle(Geom_Plane) geomXZ = new Geom_Plane(xzPlane);
            Handle(AIS_Plane) aisXZ = new AIS_Plane(geomXZ);
            aisXZ->SetSize(planeSize, planeSize);
            aisXZ->SetColor(Quantity_NOC_GREEN1);
            aisXZ->SetTransparency(0.8);
            m_context->Display(aisXZ, Standard_False);
            std::cout << "XZ plane created" << std::endl;
        }
        
        // YZ Plane (X=0, red)
        if (m_document->isPlaneVisible("planeyz")) {
            std::cout << "Creating YZ plane..." << std::endl;
            gp_Pln yzPlane(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
            Handle(Geom_Plane) geomYZ = new Geom_Plane(yzPlane);
            Handle(AIS_Plane) aisYZ = new AIS_Plane(geomYZ);
            aisYZ->SetSize(planeSize, planeSize);
            aisYZ->SetColor(Quantity_NOC_RED1);
            aisYZ->SetTransparency(0.8);
            m_context->Display(aisYZ, Standard_False);
            std::cout << "YZ plane created" << std::endl;
        }
        
        std::cout << "All planes created successfully" << std::endl;
    } catch (Standard_Failure const& e) {
        std::cerr << "Error creating planes: " << e.GetMessageString() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error creating planes" << std::endl;
    }
}

void OccViewer::fitAll() {
    // Reset camera to default view
    m_cameraDistance = 2.0f;
    m_cameraPanX = 0.0f;
    m_cameraPanY = 0.0f;
    std::cout << "Fit All" << std::endl;
}

void OccViewer::setViewFront() {
    m_cameraRotX = 0.0f;
    m_cameraRotY = 0.0f;
    m_cameraRotZ = 0.0f;
    std::cout << "View Front" << std::endl;
}

void OccViewer::setViewTop() {
    m_cameraRotX = 90.0f;
    m_cameraRotY = 0.0f;
    m_cameraRotZ = 0.0f;
    std::cout << "View Top" << std::endl;
}

void OccViewer::setViewRight() {
    m_cameraRotX = 0.0f;
    m_cameraRotY = 0.0f;
    m_cameraRotZ = -90.0f;
    std::cout << "View Right" << std::endl;
}

void OccViewer::setViewIso() {
    // Proper isometric view of XY plane:
    // Start looking down +Z at XY plane (X right, Y up)
    // Rotate 45° to the right around Y axis
    // Rotate 45° up around X axis
    m_cameraRotX = -35.264f;  // atan(1/sqrt(2)) for true isometric, ~35.264°
    m_cameraRotY = 45.0f;     // 45° rotation to the right
    m_cameraRotZ = 0.0f;      // No roll
    std::cout << "View Iso (proper isometric)" << std::endl;
}

void OccViewer::startRotation(int x, int y) {
    m_lastX = x;
    m_lastY = y;
}

void OccViewer::rotation(int x, int y) {
    int dx = x - m_lastX;
    int dy = y - m_lastY;
    
    // Update camera rotation based on mouse delta
    // Left-right controls rotation around Y axis (green, vertical)
    // Up-down controls rotation around X axis (red, horizontal)
    m_cameraRotY += dx * 0.5f;
    m_cameraRotX += dy * 0.5f;
    
    // Clamp X rotation to prevent flipping
    if (m_cameraRotX > 89.0f) m_cameraRotX = 89.0f;
    if (m_cameraRotX < -89.0f) m_cameraRotX = -89.0f;
    
    m_lastX = x;
    m_lastY = y;
}

void OccViewer::startPan(int x, int y) {
    m_lastX = x;
    m_lastY = y;
}

void OccViewer::pan(int x, int y) {
    int dx = x - m_lastX;
    int dy = y - m_lastY;
    
    // Update camera pan based on mouse delta
    float panSpeed = 0.003f;
    m_cameraPanX += dx * panSpeed;
    m_cameraPanY -= dy * panSpeed;  // Invert Y for natural movement
    
    m_lastX = x;
    m_lastY = y;
}

void OccViewer::zoom(float factor) {
    m_cameraDistance *= (1.0f - factor * 0.1f);
    
    // Clamp zoom distance - allow much closer zoom
    if (m_cameraDistance < 0.1f) m_cameraDistance = 0.1f;
    if (m_cameraDistance > 20.0f) m_cameraDistance = 20.0f;
}

void OccViewer::setHoveredPlane(const std::string& planeName) {
    m_hoveredPlane = planeName;
}

std::string OccViewer::pickPlane(int mouseX, int mouseY, int viewportWidth, int viewportHeight) {
    // Convert mouse coordinates to normalized device coordinates [-1, 1]
    float ndcX = (2.0f * mouseX) / viewportWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / viewportHeight;  // Flip Y (screen Y is top-down)
    
    // Calculate aspect ratio and orthographic bounds
    float aspect = (float)viewportWidth / viewportHeight;
    float left, right, bottom, top;
    if (aspect > 1.0f) {
        left = -aspect; right = aspect;
        bottom = -1.0f; top = 1.0f;
    } else {
        left = -1.0f; right = 1.0f;
        bottom = -1.0f / aspect; top = 1.0f / aspect;
    }
    
    // Convert NDC to clip space
    float clipX = left + (ndcX + 1.0f) * 0.5f * (right - left);
    float clipY = bottom + (ndcY + 1.0f) * 0.5f * (top - bottom);
    
    // Apply zoom to get view space position
    float viewX = clipX * m_cameraDistance;
    float viewY = clipY * m_cameraDistance;
    
    // In orthographic projection, ray origin is at the mouse position on the near plane
    // and ray direction is constant (view direction)
    // Start from far plane to ensure we're in front of all geometry
    float rayOriginX = viewX;
    float rayOriginY = viewY;
    float rayOriginZ = 100.0f;
    
    // Ray direction in view space (straight down -Z)
    float rayDirX = 0.0f;
    float rayDirY = 0.0f;
    float rayDirZ = -1.0f;
    
    // Helper functions for rotation
    auto rotateZ = [](float& x, float& y, float angleDeg) {
        float angleRad = -angleDeg * M_PI / 180.0f;  // Negative for inverse
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);
        float newX = x * cosA - y * sinA;
        float newY = x * sinA + y * cosA;
        x = newX;
        y = newY;
    };
    
    auto rotateY = [](float& x, float& z, float angleDeg) {
        float angleRad = -angleDeg * M_PI / 180.0f;  // Negative for inverse
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);
        float newX = x * cosA + z * sinA;
        float newZ = -x * sinA + z * cosA;
        x = newX;
        z = newZ;
    };
    
    auto rotateX = [](float& y, float& z, float angleDeg) {
        float angleRad = -angleDeg * M_PI / 180.0f;  // Negative for inverse
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);
        float newY = y * cosA - z * sinA;
        float newZ = y * sinA + z * cosA;
        y = newY;
        z = newZ;
    };
    
    // Apply inverse transformations in reverse order
    // Rendering order: rotZ -> rotY -> rotX -> pan
    // So inverse order: -pan -> -rotX -> -rotY -> -rotZ
    
    // Inverse pan (subtract pan offset)
    rayOriginX -= m_cameraPanX;
    rayOriginY -= m_cameraPanY;
    
    // Inverse rotation X
    rotateX(rayOriginY, rayOriginZ, m_cameraRotX);
    rotateX(rayDirY, rayDirZ, m_cameraRotX);
    
    // Inverse rotation Y
    rotateY(rayOriginX, rayOriginZ, m_cameraRotY);
    rotateY(rayDirX, rayDirZ, m_cameraRotY);
    
    // Inverse rotation Z
    rotateZ(rayOriginX, rayOriginY, m_cameraRotZ);
    rotateZ(rayDirX, rayDirY, m_cameraRotZ);
    
    // Now we have ray origin and direction in world space
    // Test intersection with each visible plane
    
    struct PlaneIntersection {
        std::string name;
        float distance;
    };
    
    std::vector<PlaneIntersection> intersections;
    float planeSize = 0.7f;
    
    // Test XY plane (Z = 0)
    if (m_document && m_document->isPlaneVisible("planexy")) {
        if (std::abs(rayDirZ) > 0.0001f) {  // Ray not parallel to plane
            float t = (0.0f - rayOriginZ) / rayDirZ;
            if (t > 0.0f) {  // Intersection in front of ray
                float hitX = rayOriginX + t * rayDirX;
                float hitY = rayOriginY + t * rayDirY;
                if (std::abs(hitX) <= planeSize && std::abs(hitY) <= planeSize) {
                    intersections.push_back({"planexy", t});
                }
            }
        }
    }
    
    // Test XZ plane (Y = 0)
    if (m_document && m_document->isPlaneVisible("planexz")) {
        if (std::abs(rayDirY) > 0.0001f) {
            float t = (0.0f - rayOriginY) / rayDirY;
            if (t > 0.0f) {
                float hitX = rayOriginX + t * rayDirX;
                float hitZ = rayOriginZ + t * rayDirZ;
                if (std::abs(hitX) <= planeSize && std::abs(hitZ) <= planeSize) {
                    intersections.push_back({"planexz", t});
                }
            }
        }
    }
    
    // Test YZ plane (X = 0)
    if (m_document && m_document->isPlaneVisible("planeyz")) {
        if (std::abs(rayDirX) > 0.0001f) {
            float t = (0.0f - rayOriginX) / rayDirX;
            if (t > 0.0f) {
                float hitY = rayOriginY + t * rayDirY;
                float hitZ = rayOriginZ + t * rayDirZ;
                if (std::abs(hitY) <= planeSize && std::abs(hitZ) <= planeSize) {
                    intersections.push_back({"planeyz", t});
                }
            }
        }
    }
    
    // Return the closest intersection
    if (!intersections.empty()) {
        std::sort(intersections.begin(), intersections.end(),
                  [](const PlaneIntersection& a, const PlaneIntersection& b) {
                      return a.distance < b.distance;
                  });
        return intersections[0].name;
    }
    
    return "";  // No intersection
}

} // namespace badcad
