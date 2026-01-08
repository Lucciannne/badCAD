#include "application.h"
#include "imgui.h"
#include <GLFW/glfw3.h>
#include <string>

namespace badcad {

void Application::renderSplash() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Fullscreen splash window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    
    ImGui::Begin("Splash", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBackground);

    // Center content
    float centerX = io.DisplaySize.x * 0.5f;
    float centerY = io.DisplaySize.y * 0.5f;

    // Placeholder for splash.png - for now show text
    ImGui::SetCursorPos(ImVec2(centerX - 100, centerY - 60));
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.85f, 0.9f, 1.0f));
    ImGui::PushFont(nullptr); // Will use custom font when loaded
    
    // Large title
    ImGui::SetWindowFontScale(3.0f);
    ImGui::Text("badCAD");
    
    // Version subtitle
    ImGui::SetCursorPos(ImVec2(centerX - 60, centerY + 20));
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("version 0.1.0");
    
    // Loading indicator
    ImGui::SetCursorPos(ImVec2(centerX - 40, centerY + 60));
    ImGui::SetWindowFontScale(1.0f);
    
    double elapsed = glfwGetTime() - m_splashStartTime;
    int dots = ((int)(elapsed * 3)) % 4;
    std::string loading = "Loading";
    for (int i = 0; i < dots; i++) loading += ".";
    ImGui::Text("%s", loading.c_str());
    
    ImGui::PopFont();
    ImGui::PopStyleColor();
    
    ImGui::End();
    ImGui::PopStyleVar(2);
}

} // namespace badcad
