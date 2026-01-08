#include "application.h"
#include "imgui.h"
#include <iostream>

namespace badcad {

void Application::renderHome() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Top toolbar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::Button("New Part", ImVec2(100, 0))) {
            std::cout << "New Part clicked" << std::endl;
            // TODO: Switch to Part Editor
        }
        
        if (ImGui::Button("New Assembly", ImVec2(120, 0))) {
            std::cout << "New Assembly clicked" << std::endl;
            // TODO: Switch to Assembly Editor
        }
        
        if (ImGui::Button("Open", ImVec2(80, 0))) {
            std::cout << "Open clicked" << std::endl;
            // TODO: Show file picker
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Recent files dropdown
        if (ImGui::BeginMenu("Recent Files")) {
            ImGui::MenuItem("(No recent files)", nullptr, false, false);
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
    
    // Main welcome pane
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - ImGui::GetFrameHeight() - 25));
    
    ImGui::Begin("Welcome", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse);
    
    // Center content
    float centerX = ImGui::GetWindowWidth() * 0.5f;
    float windowHeight = ImGui::GetWindowHeight();
    
    // Calculate logo dimensions
    float logoDisplayWidth = 200.0f;
    float logoDisplayHeight = 0.0f;
    if (m_logoTexture != 0) {
        logoDisplayHeight = (float)m_logoHeight * (logoDisplayWidth / (float)m_logoWidth);
    }
    
    // Calculate total content height to ensure it fits
    float contentHeight = logoDisplayHeight + 20 + 40 + 80 + 60 + 60 + 60; // logo + spacing + text + buttons
    float startY = (windowHeight - contentHeight) * 0.5f;
    
    // Ensure startY is never negative (add padding)
    if (startY < 20) {
        startY = 20;
    }
    
    // Display logo if available
    if (m_logoTexture != 0) {
        ImGui::SetCursorPos(ImVec2(centerX - logoDisplayWidth * 0.5f, startY));
        ImGui::Image((ImTextureID)(intptr_t)m_logoTexture, ImVec2(logoDisplayWidth, logoDisplayHeight));
        startY += logoDisplayHeight + 20;
    }
    
    // Calculate text size for proper centering
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.75f, 0.8f, 1.0f));
    ImGui::SetWindowFontScale(2.0f);
    const char* welcomeText = "Welcome to badCAD";
    ImVec2 textSize = ImGui::CalcTextSize(welcomeText);
    ImGui::SetCursorPos(ImVec2(centerX - textSize.x * 0.5f, startY));
    ImGui::Text("%s", welcomeText);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    startY += 80; // Move down for buttons
    
    // Quick action buttons
    ImGui::SetCursorPos(ImVec2(centerX - 150, startY));
    if (ImGui::Button("Create New Part", ImVec2(300, 50))) {
        std::cout << "Create New Part clicked" << std::endl;
        m_state = AppState::PartEditor;
    }
    
    ImGui::SetCursorPos(ImVec2(centerX - 150, startY + 60));
    if (ImGui::Button("Create New Assembly", ImVec2(300, 50))) {
        std::cout << "Create New Assembly clicked" << std::endl;
    }
    
    ImGui::SetCursorPos(ImVec2(centerX - 150, startY + 120));
    if (ImGui::Button("Open Existing File", ImVec2(300, 50))) {
        std::cout << "Open Existing File clicked" << std::endl;
    }
    
    // Recent files section (positioned at the end)
    float recentFilesY = startY + 200;
    ImGui::SetCursorPos(ImVec2(50, recentFilesY));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.65f, 0.7f, 1.0f));
    ImGui::Text("Recent Files");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(50, recentFilesY + 30));
    ImGui::BeginChild("RecentFiles", ImVec2(io.DisplaySize.x - 100, 150), true);
    ImGui::TextDisabled("No recent files");
    ImGui::EndChild();
    
    ImGui::End();
    
    // Bottom status bar
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 25));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 25));
    
    ImGui::Begin("StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("badCAD v0.1.0");
    ImGui::SameLine(io.DisplaySize.x - 200);
    ImGui::Text("Ready");
    
    ImGui::End();
}

} // namespace badcad
