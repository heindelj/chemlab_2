#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "renderer.h"
#include "ui_manager.h"
#include "imgui_manager.h"

struct AppData {
    UIManager *uiManager;
    Renderer *renderer;
};

// Window dimensions
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// GLFW error callback
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Callback function for window resize
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    // Update UI manager with new dimensions
    void *ptr = glfwGetWindowUserPointer(window);
    if (ptr) {
        AppData *appData = static_cast<AppData *>(ptr);
        appData->uiManager->updateScreenSize(width, height);

        // Resize framebuffers for all regions
        for (const auto &region : appData->uiManager->getRegions()) {
            appData->renderer->resizeFramebuffer(&region, width, height);
        }
    }
}

// Process input
void processInput(GLFWwindow* window) {
    // Add a static variable to track if key was pressed last frame
    static bool escapePressed = false;
    bool currentEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    
    // Only trigger once per key press (not continuously)
    if (currentEscapeState && !escapePressed) {
        glfwSetWindowShouldClose(window, true);
    }
    
    // Update key state for next frame
    escapePressed = currentEscapeState;
}

int main() {
    // Set error callback
    glfwSetErrorCallback(error_callback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);  // Ensure window has title bar and border
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);  // Make window resizable
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Molecular Viewer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Create our renderer and pass the window
    Renderer renderer(window);
    
    // Check if renderer initialized properly
    if (!renderer.initialized) {
        std::cerr << "Failed to initialize renderer. Exiting." << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // Create UI manager - will handle regions and UI components
    UIManager uiManager(SCR_WIDTH, SCR_HEIGHT);
    
    // Create AppData for callbacks
    AppData appData;
    appData.uiManager = &uiManager;
    appData.renderer = &renderer;
    glfwSetWindowUserPointer(window, &appData);

    // Set up UI regions (example: split screen into main view and sidebar)
    uiManager.addRegion("main_view", 0.0f, 0.0f, 0.8f, 1.0f); // x, y, width, height (normalized 0-1)
    uiManager.addRegion("sidebar", 0.8f, 0.0f, 0.2f, 1.0f);
    uiManager.addRegion("status", 0.0f, 0.9f, 0.8f, 0.1f);
    
    // Initialize ImGui Manager
    ImGuiManager imguiManager(window);
    if (imguiManager.init()) {
        std::cout << "ImGui initialized successfully." << std::endl;
        
        // Add example molecule info
        imguiManager.setMoleculeInfo("Caffeine", 24, 1.2f);
        imguiManager.setAppStatus("Ready to analyze molecules");
    } else {
        std::cerr << "Failed to initialize ImGui. Continuing without ImGui support." << std::endl;
    }
    
    // Enable vsync
    glfwSwapInterval(1);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window);
        
        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        imguiManager.newFrame();
        imguiManager.render();

        // Then, render all framebuffers to the screen
        for (const auto &region : uiManager.getRegions())
        {
            if (region.name == "main_view")
            {
                renderer.renderRegion(region);
                renderer.renderFramebufferToScreen(region);
            }
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Clean up
    renderer.cleanup();
    imguiManager.shutdown();
    glfwTerminate();
    return 0;
}