#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "renderer.h"
#include "ui_manager.h"
#include "imgui_manager.h"

struct AppData
{
    UIManager *uiManager;
    Renderer *renderer;
    ImGuiManager *imguiManager;
    bool mousePressed = false;
};

// Window dimensions
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

// GLFW error callback
void error_callback(int error, const char *description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Callback function for window resize
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    // Update UI manager with new dimensions
    void *ptr = glfwGetWindowUserPointer(window);
    if (ptr)
    {
        AppData *appData = static_cast<AppData *>(ptr);
        appData->uiManager->updateScreenSize(width, height);

        // Resize framebuffers for all regions
        for (const auto &region : appData->uiManager->getRegions())
        {
            appData->renderer->resizeFramebuffer(&region, width, height);
        }
    }
}

// Mouse button callback
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Get app data
    void *ptr = glfwGetWindowUserPointer(window);
    if (!ptr)
        return;

    AppData *appData = static_cast<AppData *>(ptr);

    // Pass event to ImGui first
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        return; // ImGui is using the mouse, don't handle the event
    }

    // Handle mouse button events for boundary dragging
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Try to start dragging
            if (appData->uiManager->startDragging(mouseX, mouseY))
            {
                appData->mousePressed = true;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            appData->mousePressed = false;
            appData->uiManager->endDragging();
        }
    }
}

// Cursor position callback
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    // Get app data
    void *ptr = glfwGetWindowUserPointer(window);
    if (!ptr)
        return;

    AppData *appData = static_cast<AppData *>(ptr);

    // Pass event to ImGui first
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        return; // ImGui is using the mouse, don't handle the event
    }

    if (appData->mousePressed)
    {
        // Update dragging if mouse is pressed
        appData->uiManager->updateDragging(xpos, ypos);

        // After updating regions, recreate all framebuffers to match the new sizes
        for (const auto &region : appData->uiManager->getRegions())
        {
            appData->renderer->resizeFramebuffer(&region,
                                                 appData->uiManager->screenWidth,
                                                 appData->uiManager->screenHeight);
        }
    }
    else
    {
        // Check boundaries when mouse moves (for cursor change)
        appData->uiManager->checkBoundaries(xpos, ypos);
    }
}

// Process input
void processInput(GLFWwindow *window)
{
    // Add a static variable to track if key was pressed last frame
    static bool escapePressed = false;
    bool currentEscapeState = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;

    // Only trigger once per key press (not continuously)
    if (currentEscapeState && !escapePressed)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Update key state for next frame
    escapePressed = currentEscapeState;
}

int main()
{
    // Set error callback
    glfwSetErrorCallback(error_callback);

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // Ensure window has title bar and border
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Make window resizable

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Molecular Viewer", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Create our renderer and pass the window
    Renderer renderer(window);

    // Check if renderer initialized properly
    if (!renderer.initialized)
    {
        std::cerr << "Failed to initialize renderer. Exiting." << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create UI manager - will handle regions and UI components
    UIManager uiManager(SCR_WIDTH, SCR_HEIGHT);
    uiManager.setWindow(window); // Set the window for cursor changes

    // Set up UI regions - create a 2x2 grid of regions for the triangles
    uiManager.addRegion("quad_tl", 0.0f, 0.0f, 0.495f, 0.495f);     // Top-left
    uiManager.addRegion("quad_tr", 0.505f, 0.0f, 0.495f, 0.495f);   // Top-right
    uiManager.addRegion("quad_bl", 0.0f, 0.505f, 0.495f, 0.495f);   // Bottom-left
    uiManager.addRegion("quad_br", 0.505f, 0.505f, 0.495f, 0.495f); // Bottom-right

    // Give the renderer access to the UIManager
    renderer.setUIManager(&uiManager);

    // Initialize ImGui Manager
    ImGuiManager imguiManager(window);
    if (!imguiManager.init())
    {
        std::cerr << "Failed to initialize ImGui. Continuing without ImGui support." << std::endl;
    }
    else
    {
        std::cout << "ImGui initialized successfully." << std::endl;

        // Add example molecule info
        imguiManager.setMoleculeInfo("Caffeine", 24, 1.2f);
        imguiManager.setAppStatus("Ready to analyze molecules");
    }

    // Create AppData for callbacks
    AppData appData;
    appData.uiManager = &uiManager;
    appData.renderer = &renderer;
    appData.imguiManager = &imguiManager;
    glfwSetWindowUserPointer(window, &appData);

    // Set mouse callbacks
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Enable vsync
    glfwSwapInterval(1);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Create framebuffers for all regions
    for (const auto &region : uiManager.getRegions())
    {
        renderer.createFramebufferForRegion(region);
    }

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Process input
        processInput(window);

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        imguiManager.newFrame();

        // Render all regions to their framebuffers
        for (const auto &region : uiManager.getRegions())
        {
            renderer.renderRegion(region);
        }

        // Render all framebuffers to the screen
        for (const auto &region : uiManager.getRegions())
        {
            renderer.renderFramebufferToScreen(region);
        }

        // Draw grid lines on top
        renderer.drawGridLines();

        // Render ImGui
        imguiManager.render();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    renderer.cleanupFramebuffers();
    renderer.cleanup();
    imguiManager.shutdown();
    glfwTerminate();
    return 0;
}