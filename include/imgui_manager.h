#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <string>
#include <functional>
#include <vector>

// Forward declarations
class UIManager;

class ImGuiManager {
public:
    ImGuiManager(GLFWwindow* window);
    ~ImGuiManager();
    
    // Initialize ImGui
    bool init();
    
    // Start a new frame
    void newFrame();
    
    // Render ImGui
    void render();
    
    // Cleanup
    void shutdown();
    
    // Add UI panels for different regions
    void renderSidebarUI();
    void renderStatusUI();
    
    // Property getters/setters (example for molecule viewer)
    void setMoleculeInfo(const std::string& name, int atoms, float radius);
    void setAppStatus(const std::string& status);
    
    // Callback for GLFW when window size changes
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    
    // Flag for demo window (for development)
    bool showDemoWindow = false;
    
private:
    GLFWwindow* window;
    bool initialized = false;
    
    // Application state
    struct MoleculeInfo {
        std::string name = "None";
        int atoms = 0;
        float radius = 1.0f;
    } moleculeInfo;
    
    std::string appStatus = "Ready";
    
    // UI colors and style
    void setupStyle();
};