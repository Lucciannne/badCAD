#include "application.h"
#include "part_editor.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace badcad {

// Static callback wrappers
static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->onFramebufferResize(width, height);
    }
}

static void windowRefreshCallback(GLFWwindow* window) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->render();
    }
}

Application::Application() {
    m_partEditor = std::make_unique<PartEditor>(this);
}

Application::~Application() {
    shutdown();
}

bool Application::init() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // OpenGL 3.3 - use Compatibility Profile for legacy OpenGL support
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Create window
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, 
                                 m_windowTitle.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync
    
    // Set window user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);
    
    // Set callbacks for continuous rendering during resize
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetWindowRefreshCallback(m_window, windowRefreshCallback);
    
    loadWindowIcon();

    setupImGui();
    loadLogoImage();

    m_splashStartTime = glfwGetTime();
    m_initialized = true;

    std::cout << "badCAD initialized successfully" << std::endl;
    return true;
}

void Application::setupImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Load custom font
    io.Fonts->AddFontFromFileTTF("resources/fonts/IBMPlexSans-Regular.ttf", 16.0f);

    // Setup style - dark theme for CAD app
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 3.0f;
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void Application::run() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        // Auto-transition from splash after 1 second
        if (m_state == AppState::Splash) {
            double elapsed = glfwGetTime() - m_splashStartTime;
            if (elapsed >= 1.0) {
                setState(AppState::Home);
            }
        }

        render();
    }
}

void Application::render() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render current state
    switch (m_state) {
        case AppState::Splash:
            renderSplash();
            break;
        case AppState::Home:
            renderHome();
            break;
        case AppState::PartEditor:
            if (m_partEditor) {
                m_partEditor->render();
            }
            break;
        case AppState::AssemblyEditor:
            // TODO: Implement assembly editor
            break;
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    
    // Clear with appropriate background
    if (m_state == AppState::Splash) {
        glClearColor(0.15f, 0.15f, 0.18f, 1.0f); // Dark blue-gray splash
    } else {
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f); // Darker background for main app
    }
    
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
}

void Application::onFramebufferResize(int width, int height) {
    // Update viewport on resize
    glViewport(0, 0, width, height);
    // Trigger a render to update the display immediately
    render();
}

void Application::setState(AppState newState) {
    m_state = newState;
    std::cout << "State changed to: " << (int)newState << std::endl;
}

void Application::shutdown() {
    if (m_initialized) {
        // Clean up logo texture
        if (m_logoTexture != 0) {
            glDeleteTextures(1, &m_logoTexture);
            m_logoTexture = 0;
        }
        
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();

        m_initialized = false;
    }
}

void Application::loadLogoImage() {
    const char* logoPath = "resources/icons/logo_hq.png";
    
    // Load image using stb_image
    int channels;
    unsigned char* data = stbi_load(logoPath, &m_logoWidth, &m_logoHeight, &channels, 4);
    
    if (data == nullptr) {
        std::cerr << "Failed to load logo image: " << logoPath << std::endl;
        return;
    }
    
    // Create OpenGL texture
    glGenTextures(1, &m_logoTexture);
    glBindTexture(GL_TEXTURE_2D, m_logoTexture);
    
    // Set texture parameters (0x812F is GL_CLAMP_TO_EDGE)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_logoWidth, m_logoHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // Free image data
    stbi_image_free(data);
    
    std::cout << "Logo loaded: " << m_logoWidth << "x" << m_logoHeight << std::endl;
}

void Application::loadWindowIcon() {
    const char* iconPath = "resources/icons/logo_black_bg.png";
    
    // Load icon using stb_image
    int width, height, channels;
    unsigned char* data = stbi_load(iconPath, &width, &height, &channels, 4);
    
    if (data == nullptr) {
        std::cerr << "Failed to load window icon: " << iconPath << std::endl;
        return;
    }
    
    // Set window icon
    GLFWimage icon;
    icon.width = width;
    icon.height = height;
    icon.pixels = data;
    
    glfwSetWindowIcon(m_window, 1, &icon);
    
    // Free image data
    stbi_image_free(data);
    
    std::cout << "Window icon loaded: " << width << "x" << height << std::endl;
}

} // namespace badcad
