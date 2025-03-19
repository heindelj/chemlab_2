#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <map>
#include "ui_region.h"

class Renderer {
public:
    Renderer(GLFWwindow* window);
    bool initialized = false;
    void cleanup();
    
    void renderRegion(const UIRegion& region);

    // Framebuffer methods and type
    void createFramebufferForRegion(const UIRegion &region);
    void bindFramebufferForRegion(const std::string &regionName);
    void unbindFramebuffer();
    void renderFramebufferToScreen(const UIRegion &region);
    void resizeFramebuffer(const UIRegion* region, int width, int height);
    void cleanupFramebuffers();
    // Draw boundary lines between UI regions
    void renderBoundaryLines(const std::vector<UIRegion> &regions, float lineWidth = 2.0f, glm::vec3 lineColor = glm::vec3(0.3f, 0.3f, 0.3f));

    struct FramebufferObject {
        unsigned int fbo;          // Framebuffer object
        unsigned int colorTexture; // Color attachment
        unsigned int depthRbo;     // Depth renderbuffer
        int width;                 // Width in pixels
        int height;                // Height in pixels
    };
    std::map<std::string, FramebufferObject> framebuffers;

    // Future methods for specialized rendering
    void renderMolecule(const UIRegion& region); /* Molecule data */
    void renderGraph(const UIRegion& region); /* Graph data */
    void renderControls(const UIRegion& region);
    
    // Clear the whole framebuffer
    void clearFrame(float r = 0.1f, float g = 0.1f, float b = 0.1f, float a = 1.0f);
    
    // Set the background color for all rendering
    void setBackgroundColor(float r, float g, float b);

    // Shaders
    unsigned int basicShaderProgram;
    unsigned int triangleShaderProgram;
    unsigned int framebufferShaderProgram;
    unsigned int lineShaderProgram;

    // Reference to the window
    GLFWwindow *window;

    // Setup methods
    void initShaders();
    
    // Utility methods
    unsigned int compileShader(unsigned int type, const char* source);
    unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);
    
    // Background color
    glm::vec3 backgroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
};