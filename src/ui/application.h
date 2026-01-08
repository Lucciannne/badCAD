#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <memory>

namespace badcad {

class PartEditor;

enum class AppState {
    Splash,
    Home,
    PartEditor,
    AssemblyEditor
};

class Application {
public:
    Application();
    ~Application();

    bool init();
    void run();
    void shutdown();

    void setState(AppState newState);
    AppState getState() const { return m_state; }

    GLFWwindow* getWindow() { return m_window; }
    
    // Public for GLFW callbacks
    void onFramebufferResize(int width, int height);
    void render();

private:
    void setupImGui();
    void renderSplash();
    void renderHome();
    void loadLogoImage();
    void loadWindowIcon();

    GLFWwindow* m_window = nullptr;
    AppState m_state = AppState::Splash;
    double m_splashStartTime = 0.0;
    bool m_initialized = false;

    // Window properties
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    std::string m_windowTitle = "badCAD v0.1.0";
    
    // Logo texture
    unsigned int m_logoTexture = 0;
    int m_logoWidth = 0;
    int m_logoHeight = 0;
    
    // Editors
    std::unique_ptr<PartEditor> m_partEditor;
};

} // namespace badcad
