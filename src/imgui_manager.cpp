#include "imgui_manager.h"
#include "ui_manager.h"
#include "renderer.h"
#include <iostream>

GLFWmousebuttonfun ImGuiManager::OrigMouseButtonCallback = nullptr;
GLFWcursorposfun ImGuiManager::OrigCursorPosCallback = nullptr;
GLFWscrollfun ImGuiManager::OrigScrollCallback = nullptr;
GLFWkeyfun ImGuiManager::OrigKeyCallback = nullptr;
GLFWcharfun ImGuiManager::OrigCharCallback = nullptr;

void ImGuiManager::ImGuiMouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    // Process through ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    // Check if ImGui wants to use this event
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && OrigMouseButtonCallback != nullptr)
    {
        OrigMouseButtonCallback(window, button, action, mods);
    }
}

void ImGuiManager::ImGuiCursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    // Process through ImGui
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    // Check if ImGui wants to use this event
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && OrigCursorPosCallback != nullptr)
    {
        OrigCursorPosCallback(window, xpos, ypos);
    }
}

void ImGuiManager::ImGuiScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    // Process through ImGui
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    // Check if ImGui wants to use this event
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && OrigScrollCallback != nullptr)
    {
        OrigScrollCallback(window, xoffset, yoffset);
    }
}

void ImGuiManager::ImGuiKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Process through ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    // Check if ImGui wants to use this event
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard && OrigKeyCallback != nullptr)
    {
        OrigKeyCallback(window, key, scancode, action, mods);
    }
}

void ImGuiManager::ImGuiCharCallback(GLFWwindow *window, unsigned int c)
{
    // Process through ImGui
    ImGui_ImplGlfw_CharCallback(window, c);

    // Check if ImGui wants to use this event
    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard && OrigCharCallback != nullptr)
    {
        OrigCharCallback(window, c);
    }
}

// Forward declaration of AppData structure from main.cpp
struct AppData
{
    UIManager *uiManager;
    Renderer *renderer;
    ImGuiManager *imguiManager;
    bool mousePressed = false;
};

ImGuiManager::ImGuiManager(GLFWwindow *window) : window(window)
{
}

ImGuiManager::~ImGuiManager()
{
    shutdown();
}

bool ImGuiManager::init()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    // Enable keyboard controls, docking, and viewports
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 460");

    // Setup style
    setupStyle();

    initialized = true;
    return true;
}

void ImGuiManager::setupStyle()
{
    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();

    // Adjust colors for a more modern look
    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.27f, 0.85f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);

    // Adjust style for more padding and rounded corners
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowRounding = 6.0f;
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 4);
}

void ImGuiManager::newFrame()
{
    if (!initialized)
        return;

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create an invisible dockspace (we're using our custom regions)
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                    ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking |
                                    ImGuiWindowFlags_NoDecoration;

    // We need to push these style vars to remove window padding and make it truly invisible
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    // Begin the invisible window
    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    // Configure and add the dockspace
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                         ImGuiDockNodeFlags_NoDockingOverCentralNode;
    ImGuiID dockspace_id = ImGui::GetID("InvisibleDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    ImGui::End();

    // Show demo window if enabled
    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // Always render our UI components
    renderSidebarUI();
    renderStatusUI();
}

void ImGuiManager::render()
{
    if (!initialized)
        return;

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiManager::shutdown()
{
    if (initialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        initialized = false;
    }
}

void ImGuiManager::renderSidebarUI()
{
    // Get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Get sidebar region position and size - Use the dynamic positions
    const UIRegion *sidebarRegion = nullptr;

    // Try to find the sidebar region in the window user pointer
    void *ptr = glfwGetWindowUserPointer(window);
    if (ptr)
    {
        AppData *appData = static_cast<AppData *>(ptr);
        if (appData && appData->uiManager)
        {
            sidebarRegion = appData->uiManager->getRegion("sidebar");
        }
    }

    if (!sidebarRegion)
    {
        // Fallback to default position if we can't find the dynamic region
        ImGui::SetNextWindowPos(ImVec2(width * 0.8f, 0));
        ImGui::SetNextWindowSize(ImVec2(width * 0.2f, height));
    }
    else
    {
        // Use dynamic positions from the UIManager
        ImGui::SetNextWindowPos(ImVec2(sidebarRegion->x * width, sidebarRegion->y * height));
        ImGui::SetNextWindowSize(ImVec2(sidebarRegion->width * width, sidebarRegion->height * height));
    }

    // Create sidebar window without typical window decorations
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Sidebar", nullptr, flags))
    {
        // Sidebar header
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::Text("Molecule Explorer");
        ImGui::Separator();
        ImGui::PopFont();

        // File operations
        if (ImGui::CollapsingHeader("File Operations", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Open Molecule", ImVec2(-1, 0)))
            {
                // Will implement file loading later
            }

            if (ImGui::Button("Save Image", ImVec2(-1, 0)))
            {
                // Will implement screenshot saving later
            }
        }

        // Visualization options
        if (ImGui::CollapsingHeader("Visualization", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const char *renderModes[] = {"Ball and Stick", "Space Filling", "Wireframe", "Ribbon"};
            static int renderModeIndex = 0;
            ImGui::Combo("Render Mode", &renderModeIndex, renderModes, IM_ARRAYSIZE(renderModes));

            // Color schemes
            const char *colorSchemes[] = {"Element", "Residue", "Chain", "Temperature"};
            static int colorSchemeIndex = 0;
            ImGui::Combo("Color Scheme", &colorSchemeIndex, colorSchemes, IM_ARRAYSIZE(colorSchemes));

            // Background color
            static ImVec4 bgColor = ImVec4(0.2f, 0.3f, 0.3f, 1.0f); // Default bg color
            if (ImGui::ColorEdit3("Background", (float *)&bgColor))
            {
                // Will update background color later
            }
        }

        // Analysis tools with ImPlot
        if (ImGui::CollapsingHeader("Analysis", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Measure Distance", ImVec2(-1, 0)))
            {
                // Will implement distance measurement later
            }

            if (ImGui::Button("Calculate RMSD", ImVec2(-1, 0)))
            {
                // Will implement RMSD calculation later
            }

            // Demo plot with ImPlot
            if (ImGui::TreeNode("Energy Analysis"))
            {
                // Sample data for the plot
                static float x_data[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
                static float y_data[] = {0.0f, 0.8f, 0.4f, 1.2f, 0.9f, 0.6f};

                // Create a simple plot
                if (ImPlot::BeginPlot("Energy vs. Time", ImVec2(-1, 200)))
                {
                    ImPlot::PlotLine("Potential Energy", x_data, y_data, 6);
                    ImPlot::EndPlot();
                }

                ImGui::TreePop();
            }
        }

        // Settings
        if (ImGui::CollapsingHeader("Settings"))
        {
            ImGui::Checkbox("Show ImGui Demo", &showDemoWindow);

            // Show ImPlot demo window option
            static bool showImPlotDemo = false;
            ImGui::Checkbox("Show ImPlot Demo", &showImPlotDemo);
            if (showImPlotDemo)
            {
                ImPlot::ShowDemoWindow(&showImPlotDemo);
            }

            // Performance settings
            static bool vsync = true;
            if (ImGui::Checkbox("VSync", &vsync))
            {
                glfwSwapInterval(vsync ? 1 : 0);
            }
        }
    }
    ImGui::End();
}

void ImGuiManager::renderStatusUI()
{
    // Get current window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Get status region position and size - Use the dynamic positions
    const UIRegion *statusRegion = nullptr;

    // Try to find the status region in the window user pointer
    void *ptr = glfwGetWindowUserPointer(window);
    if (ptr)
    {
        AppData *appData = static_cast<AppData *>(ptr);
        if (appData && appData->uiManager)
        {
            statusRegion = appData->uiManager->getRegion("status");
        }
    }

    if (!statusRegion)
    {
        // Fallback to default position if we can't find the dynamic region
        ImGui::SetNextWindowPos(ImVec2(0, height - 24));
        ImGui::SetNextWindowSize(ImVec2(width * 0.8f, 24));
    }
    else
    {
        // Use dynamic positions from the UIManager
        ImGui::SetNextWindowPos(ImVec2(statusRegion->x * width, statusRegion->y * height));
        ImGui::SetNextWindowSize(ImVec2(statusRegion->width * width, statusRegion->height * height));
    }

    ImGuiWindowFlags statusFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 3));

    if (ImGui::Begin("StatusBar", nullptr, statusFlags))
    {
        // Left side - status message
        ImGui::Text("%s", appStatus.c_str());

        // Right side - fps counter
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }

    // Debug info
    ImGui::SameLine(ImGui::GetWindowWidth() - 480);
    ImGuiIO &io = ImGui::GetIO();
    ImGui::Text("ImGui WantCaptureMouse: %s", io.WantCaptureMouse ? "true" : "false");

    ImGui::End();
    ImGui::PopStyleVar();
}

void ImGuiManager::setMoleculeInfo(const std::string &name, int atoms, float radius)
{
    moleculeInfo.name = name;
    moleculeInfo.atoms = atoms;
    moleculeInfo.radius = radius;
}

void ImGuiManager::setAppStatus(const std::string &status)
{
    appStatus = status;
}

void ImGuiManager::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    // This callback will be called in addition to the main app's framebuffer callback
    // GLFW will call both the original callback and this one
    glViewport(0, 0, width, height);
}