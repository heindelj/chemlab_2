#include "renderer.h"
#include <iostream>

// Basic vertex shader
const char* basicVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    
    out vec3 vertexColor;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        vertexColor = aColor;
    }
)";

// Basic fragment shader
const char* basicFragmentShaderSource = R"(
    #version 330 core
    in vec3 vertexColor;
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(vertexColor, 1.0);
    }
)";

Renderer::Renderer(GLFWwindow* window) : window(window) {
    initShaders();
}

void Renderer::cleanup() {
    // Only delete the shader program if it's a valid ID
    if (basicShaderProgram > 0) {
        glDeleteProgram(basicShaderProgram);
    }
}

void Renderer::initShaders() {
    try {
        basicShaderProgram = createShaderProgram(basicVertexShaderSource, basicFragmentShaderSource);
        if (basicShaderProgram > 0) {
            initialized = true;
        } else {
            std::cerr << "Failed to create shader program." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in initShaders: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception in initShaders" << std::endl;
    }
}

void Renderer::renderRegion(const UIRegion& region) {
    if (!initialized) {
        std::cerr << "Renderer not properly initialized, cannot render." << std::endl;
        return;
    }
    
    // Set viewport based on region dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    int regionX = static_cast<int>(region.x * width);
    int regionY = static_cast<int>(region.y * height);
    int regionWidth = static_cast<int>(region.width * width);
    int regionHeight = static_cast<int>(region.height * height);
    
    glViewport(regionX, regionY, regionWidth, regionHeight);
    
    // Draw region boundary (for debugging)
    glUseProgram(basicShaderProgram);
    
    // For now, just draw a colored rectangle to show the region
    // In the future, this will dispatch to specialized rendering methods based on region content
    
    // Draw based on region type/content
    if (region.name == "main_view") {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // Will call renderMolecule() when implemented
    }
    else if (region.name == "sidebar") {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        // Will call renderControls() when implemented
    }
    else if (region.name == "status") {
        glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
        // Will display status info
    }
    
    // This is just placeholder drawing code
    // Later we'll replace with actual geometry rendering
}

unsigned int Renderer::compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    if (shader == 0) {
        std::cerr << "Failed to create shader" << std::endl;
        return 0;
    }
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

unsigned int Renderer::createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = 0, fragmentShader = 0, shaderProgram = 0;
    int success;
    char infoLog[512];
    
    try {
        // Compile vertex shader
        vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0) {
            std::cerr << "Failed to compile vertex shader" << std::endl;
            return 0;
        }
        
        // Compile fragment shader
        fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (fragmentShader == 0) {
            glDeleteShader(vertexShader);
            std::cerr << "Failed to compile fragment shader" << std::endl;
            return 0;
        }
        
        // Create and link shader program
        shaderProgram = glCreateProgram();
        if (shaderProgram == 0) {
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            std::cerr << "Failed to create shader program" << std::endl;
            return 0;
        }
        
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        
        // Check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(shaderProgram);
            return 0;
        }
        
        // Delete shaders as they're linked into the program and no longer necessary
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return shaderProgram;
    }
    catch (const std::exception& e) {
        // Clean up in case of exception
        if (vertexShader > 0) glDeleteShader(vertexShader);
        if (fragmentShader > 0) glDeleteShader(fragmentShader);
        if (shaderProgram > 0) glDeleteProgram(shaderProgram);
        
        std::cerr << "Exception in createShaderProgram: " << e.what() << std::endl;
        return 0;
    }
}