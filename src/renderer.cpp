#include "renderer.h"
#include <iostream>

// Basic vertex shader
const char* basicVertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
    
    out vec4 vertexColor; // specify a color output to the fragment shader

    void main()
    {
        gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
        vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color
    }
)";

// Basic fragment shader
const char* basicFragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
    in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

    void main()
    {
        FragColor = vertexColor;
    }
)";

// Basic triangle shader
const char* triangleVertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

    void main()
    {
        gl_Position = vec4(aPos, 1.0);
    }
)";

// Basic fragment shader
const char* triangleFragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
  
    uniform vec4 u_Color;

    void main()
    {
        FragColor = u_Color;
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
    basicShaderProgram = createShaderProgram(basicVertexShaderSource, basicFragmentShaderSource);
    triangleShaderProgram = createShaderProgram(triangleVertexShaderSource, triangleFragmentShaderSource);
}

void Renderer::clearFrame(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setBackgroundColor(float r, float g, float b) {
    backgroundColor = glm::vec3(r, g, b);
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
    
    // Draw based on region type/content
    if (region.name == "main_view") {
        // Main view background (ImGui will draw on top)

        // The below is just placeholder to put something on the screen while
        // we get the rendering code behaving as we want.
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };
        
        unsigned int VBO, VAO;
        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(triangleShaderProgram);
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
        glBindVertexArray(VAO);

        float timeValue = glfwGetTime();
        float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        int vertexColorLocation = glGetUniformLocation(triangleShaderProgram, "u_Color");
        glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // Will call renderMolecule() when implemented
    }
    else if (region.name == "sidebar") {
        // Since we're using ImGui for the sidebar, just clear to match background
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    }
    else if (region.name == "status") {
        // Since we're using ImGui for the status bar, just clear to match background
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
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
    
    // Compile vertex and fragment shaders
    vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // Create actual shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success != 1) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return 0;
    } else {
        initialized = true;
    }
    
    // Delete shaders as they're linked into the program and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}