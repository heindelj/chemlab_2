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

    bool initialized;

    // Shaders
    unsigned int basicShaderProgram;
    
    // Reference to the window
    GLFWwindow* window;
    
    // Setup methods
    void initShaders();
    
    // Utility methods
    unsigned int compileShader(unsigned int type, const char* source);
    unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource);
};