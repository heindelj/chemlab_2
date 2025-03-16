#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "ui_region.h"

class Renderer {
public:
    Renderer(GLFWwindow* window);
    void cleanup();
    
    void renderRegion(const UIRegion& region);
    
    // Future methods for specialized rendering
    void renderMolecule(const UIRegion& region); /* Molecule data */
    void renderGraph(const UIRegion& region); /* Graph data */
    void renderControls(const UIRegion& region);
    
    // Clear the whole framebuffer
    void clearFrame(float r = 0.1f, float g = 0.1f, float b = 0.1f, float a = 1.0f);
    
    // Set the background color for all rendering
    void setBackgroundColor(float r, float g, float b);

    bool initialized = false;

    // Shaders
    unsigned int basicShaderProgram;
    unsigned int triangleShaderProgram;
    
    // Reference to the window
    GLFWwindow* window;
    
    // Setup methods
    void initShaders();
    
    // Utility methods
    unsigned int compileShader(unsigned int type, const char* source);
    unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);
    
    // Background color
    glm::vec3 backgroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
};