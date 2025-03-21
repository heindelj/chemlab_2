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

    // Add these to the public section of ImGuiManager class
    static void ImGuiMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void ImGuiCursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void ImGuiScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void ImGuiKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void ImGuiCharCallback(GLFWwindow *window, unsigned int c);

    // Original application callbacks that we'll chain to
    static GLFWmousebuttonfun OrigMouseButtonCallback;
    static GLFWcursorposfun OrigCursorPosCallback;
    static GLFWscrollfun OrigScrollCallback;
    static GLFWkeyfun OrigKeyCallback;
    static GLFWcharfun OrigCharCallback;

    // Set up the callbacks
    static void InstallCallbacks(GLFWwindow *window);

    // Callback for GLFW when window size changes
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    // Flag for demo window (for development)
    bool showDemoWindow = false;
    
    GLFWwindow* window;
    bool initialized = false;
    
    // Application state
    struct MoleculeInfo {
        std::string name = "None";
        int atoms = 0;
        float radius = 1.0f;
    } moleculeInfo;

    struct BoundaryLineSettings {
        bool show = true;
        float width = 2.0f;
        ImVec4 color = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    } boundaryLineSettings;
    const BoundaryLineSettings &getBoundaryLineSettings() const { return boundaryLineSettings; }

    std::string appStatus = "Ready";
    
    // UI colors and style
    void setupStyle();
};