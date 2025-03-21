#include "renderer.h"
#include "ui_manager.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

// Forward declaration of AppData
struct AppData
{
    class UIManager *uiManager;
    class Renderer *renderer;
    class ImGuiManager *imguiManager;
    bool mousePressed = false;
};

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

// Framebuffer vertex shader
const char *framebufferVertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    out vec2 TexCoord;
    
    void main()
    {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

// Framebuffer fragment shader
const char *framebufferFragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
    
    in vec2 TexCoord;
    
    uniform sampler2D framebufferTexture;
    
    void main()
    {
        FragColor = texture(framebufferTexture, TexCoord);
    }
)";

const char *lineVertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 uProjection;
    
    void main()
    {
        gl_Position = uProjection * vec4(aPos, 1.0);
    }
)";

const char *lineFragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
    
    uniform vec3 uColor;
    
    void main()
    {
        FragColor = vec4(uColor, 1.0);
    }
)";

Renderer::Renderer(GLFWwindow* window) : window(window) {
    initShaders();
    // This will be a loop over shaders eventually...
    if (basicShaderProgram != 0 && triangleShaderProgram != 0 &&
        framebufferShaderProgram != 0 && lineShaderProgram != 0)
        initialized = true;
    initialized = true;
}

void Renderer::cleanup() {
    // TODO: Obviously should write a shader abstraction and get an array of Shaders.
    // Delete shader programs if they're valid IDs
    if (basicShaderProgram > 0)
    {
        glDeleteProgram(basicShaderProgram);
    }

    if (triangleShaderProgram > 0)
    {
        glDeleteProgram(triangleShaderProgram);
    }

    if (framebufferShaderProgram > 0)
    {
        glDeleteProgram(framebufferShaderProgram);
    }

    if (lineShaderProgram > 0)
    {
        glDeleteProgram(lineShaderProgram);
    }

    // Clean up framebuffers
    cleanupFramebuffers();
}

void Renderer::initShaders() {
    basicShaderProgram = createShaderProgram(basicVertexShaderSource, basicFragmentShaderSource);
    triangleShaderProgram = createShaderProgram(triangleVertexShaderSource, triangleFragmentShaderSource);
    framebufferShaderProgram = createShaderProgram(framebufferVertexShaderSource, framebufferFragmentShaderSource);
    lineShaderProgram = createShaderProgram(lineVertexShaderSource, lineFragmentShaderSource);
}

void Renderer::clearFrame(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setBackgroundColor(float r, float g, float b) {
    backgroundColor = glm::vec3(r, g, b);
}

void Renderer::renderBoundaryLines(const std::vector<UIRegion> &regions, float lineWidth, glm::vec3 lineColor) {
    // Get window dimensions
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    // Save current OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    GLfloat originalLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &originalLineWidth);

    // Disable depth testing for 2D lines and set line width
    glDisable(GL_DEPTH_TEST);
    glLineWidth(lineWidth);

    // Use line shader program
    glUseProgram(lineShaderProgram);

    // Set up orthographic projection matrix (2D)
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth),
                                      static_cast<float>(windowHeight), 0.0f,
                                      -1.0f, 1.0f);

    // Set shader uniforms
    unsigned int projectionLoc = glGetUniformLocation(lineShaderProgram, "uProjection");
    unsigned int colorLoc = glGetUniformLocation(lineShaderProgram, "uColor");

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(colorLoc, 1, glm::value_ptr(lineColor));

    // Create VAO and VBO for lines
    unsigned int lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    // Bind VAO and VBO
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

    // Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // For each region, draw its boundaries
    for (const auto &region : regions)
    {
        // Calculate screen coordinates
        float x1 = region.x * windowWidth;
        float y1 = region.y * windowHeight;
        float x2 = (region.x + region.width) * windowWidth;
        float y2 = (region.y + region.height) * windowHeight;

        // Define the 4 lines of the rectangle (8 vertices, 4 lines)
        float lines[] = {
            // Line 1: top edge
            x1, y1, 0.0f,
            x2, y1, 0.0f,

            // Line 2: right edge
            x2, y1, 0.0f,
            x2, y2, 0.0f,

            // Line 3: bottom edge
            x2, y2, 0.0f,
            x1, y2, 0.0f,

            // Line 4: left edge
            x1, y2, 0.0f,
            x1, y1, 0.0f};

        // Upload line data
        glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_DYNAMIC_DRAW);

        // Draw lines
        glDrawArrays(GL_LINES, 0, 8);
    }

    // Clean up
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);

    // Restore original OpenGL state
    if (depthTestEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    glLineWidth(originalLineWidth);
}

void Renderer::renderRegion(const UIRegion& region) {
    // Check if a framebuffer exists for this region, create one if not
    if (framebuffers.find(region.name) == framebuffers.end())
    {
        createFramebufferForRegion(region);
    }

    // Bind the framebuffer for this region
    bindFramebufferForRegion(region.name);

    // Clear the framebuffer
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render content to the framebuffer
    if (region.name == "main_view")
    {
        // The below is just placeholder to put something on the screen while
        // we get the rendering code behaving as we want.
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f
        };

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(triangleShaderProgram);

        // Set time and rotation direction (clockwise)
        float timeValue = glfwGetTime();
        int timeLocation = glGetUniformLocation(triangleShaderProgram, "u_Time");
        int directionLocation = glGetUniformLocation(triangleShaderProgram, "u_Direction");
        glUniform1f(timeLocation, timeValue);
        glUniform1f(directionLocation, 1.0f);

        // Set color to green with animation
        float greenPulse = (sin(timeValue) / 5.0f) + 0.7f; // Pulse between 0.5 and 0.9
        int colorLocation = glGetUniformLocation(triangleShaderProgram, "u_Color");
        glUniform4f(colorLocation, 0.0f, greenPulse, 0.2f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Clean up
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    if (region.name == "quad_tl")
    {
        // Top-left: Green upward triangle
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f};

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(triangleShaderProgram);

        // Set color to green
        int colorLocation = glGetUniformLocation(triangleShaderProgram, "u_Color");
        glUniform4f(colorLocation, 0.0f, 0.8f, 0.2f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Clean up
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    else if (region.name == "quad_tr")
    {
        // Top-right: Red right-pointing triangle
        float vertices[] = {
            -0.5f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, 0.0f, 0.0f};

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(triangleShaderProgram);

        // Set color to red
        int colorLocation = glGetUniformLocation(triangleShaderProgram, "u_Color");
        glUniform4f(colorLocation, 0.9f, 0.1f, 0.1f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Clean up
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    else if (region.name == "quad_bl")
    {
        // Bottom-left: Blue downward triangle
        float vertices[] = {
            -0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            0.0f, -0.5f, 0.0f};

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(triangleShaderProgram);

        // Set color to blue
        int colorLocation = glGetUniformLocation(triangleShaderProgram, "u_Color");
        glUniform4f(colorLocation, 0.1f, 0.3f, 0.9f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Clean up
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    else if (region.name == "quad_br")
    {
        // Bottom-right: Yellow left-pointing triangle
        float vertices[] = {
            0.5f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, 0.0f, 0.0f};

        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glUseProgram(triangleShaderProgram);

        // Set color to yellow
        int colorLocation = glGetUniformLocation(triangleShaderProgram, "u_Color");
        glUniform4f(colorLocation, 0.9f, 0.9f, 0.1f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Clean up
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
    else if (region.name == "sidebar")
    {
        // Clear with background color - ImGui will render on top later
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    }
    else if (region.name == "status")
    {
        // Clear with background color - ImGui will render on top later
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    }

    // Unbind the framebuffer when done
    unbindFramebuffer();

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
    }
    
    // Delete shaders as they're linked into the program and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void Renderer::createFramebufferForRegion(const UIRegion &region)
{
    // Get window dimensions to calculate appropriate framebuffer size
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    // Calculate framebuffer dimensions based on region size
    int fbWidth = static_cast<int>(region.width * windowWidth);
    int fbHeight = static_cast<int>(region.height * windowHeight);

    // Create a new framebuffer object
    FramebufferObject fbo;
    fbo.width = fbWidth;
    fbo.height = fbHeight;

    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fbo.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);

    // Create a color attachment texture
    glGenTextures(1, &fbo.colorTexture);
    glBindTexture(GL_TEXTURE_2D, fbo.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.colorTexture, 0);

    // Create a renderbuffer object for depth
    glGenRenderbuffers(1, &fbo.depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo.depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbWidth, fbHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depthRbo);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete for region: " << region.name << std::endl;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Store the framebuffer in our map
    framebuffers[region.name] = fbo;

    std::cout << "Created framebuffer for region: " << region.name
              << " (" << fbWidth << "x" << fbHeight << ")" << std::endl;
}

void Renderer::bindFramebufferForRegion(const std::string &regionName)
{
    auto it = framebuffers.find(regionName);
    if (it != framebuffers.end())
    {
        glBindFramebuffer(GL_FRAMEBUFFER, it->second.fbo);
        glViewport(0, 0, it->second.width, it->second.height);
    }
    else
    {
        std::cerr << "No framebuffer exists for region: " << regionName << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer
    }
}

void Renderer::unbindFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Restore default viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
}

void Renderer::renderFramebufferToScreen(const UIRegion &region)
{
    auto it = framebuffers.find(region.name);
    if (it == framebuffers.end())
    {
        std::cerr << "No framebuffer exists for region: " << region.name << std::endl;
        return;
    }

    // Get window dimensions
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    // Calculate viewport based on region dimensions
    int x = static_cast<int>(region.x * windowWidth);
    int y = static_cast<int>((1.0f - region.y - region.height) * windowHeight); // Flip Y coordinate
    int width = static_cast<int>(region.width * windowWidth);
    int height = static_cast<int>(region.height * windowHeight);

    // Ensure we don't have zero-sized viewports
    width = std::max(1, width);
    height = std::max(1, height);

    // Set viewport to the region
    glViewport(x, y, width, height);

    // Disable depth testing temporarily for 2D rendering
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    // Enable blending for transparent regions
    GLboolean blendEnabled;
    glGetBooleanv(GL_BLEND, &blendEnabled);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Simple fullscreen quad for rendering the texture
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;

    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }

    // Use the framebuffer shader program
    glUseProgram(framebufferShaderProgram);

    // Set the texture uniform
    glUniform1i(glGetUniformLocation(framebufferShaderProgram, "framebufferTexture"), 0);

    // Bind the texture from the framebuffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, it->second.colorTexture);

    // Draw quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    // Restore blend state
    if (!blendEnabled)
    {
        glDisable(GL_BLEND);
    }

    // Restore depth testing state
    if (depthTestEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
}

void Renderer::resizeFramebuffer(const UIRegion* region, int windowWidth, int windowHeight)
{
    auto it = framebuffers.find(region->name);
    if (it == framebuffers.end())
    {
        return;
    }

    if (!region)
    {
        std::cerr << "Could not find region" << std::endl;
        return;
    }

    // Calculate new framebuffer size
    int newWidth = static_cast<int>(region->width * windowWidth);
    int newHeight = static_cast<int>(region->height * windowHeight);

    // If size hasn't changed, do nothing
    if (newWidth == it->second.width && newHeight == it->second.height)
    {
        return;
    }

    // Clean up old framebuffer resources
    glDeleteTextures(1, &it->second.colorTexture);
    glDeleteRenderbuffers(1, &it->second.depthRbo);
    glDeleteFramebuffers(1, &it->second.fbo);

    // Create new framebuffer with updated size
    FramebufferObject &fbo = it->second;
    fbo.width = newWidth;
    fbo.height = newHeight;

    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fbo.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);

    // Create a color attachment texture
    glGenTextures(1, &fbo.colorTexture);
    glBindTexture(GL_TEXTURE_2D, fbo.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newWidth, newHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.colorTexture, 0);

    // Create a renderbuffer object for depth
    glGenRenderbuffers(1, &fbo.depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo.depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, newWidth, newHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.depthRbo);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Resized framebuffer is not complete for region: " << region->name << std::endl;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "Resized framebuffer for region: " << region->name
              << " to " << newWidth << "x" << newHeight << std::endl;
}

void Renderer::cleanupFramebuffers()
{
    for (auto &pair : framebuffers)
    {
        FramebufferObject &fbo = pair.second;
        glDeleteTextures(1, &fbo.colorTexture);
        glDeleteRenderbuffers(1, &fbo.depthRbo);
        glDeleteFramebuffers(1, &fbo.fbo);
    }
    framebuffers.clear();
}

void Renderer::drawGridLines() {

    if (!uiManager)
        return;

    // Disable depth testing temporarily
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    // Save current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Set viewport to entire window
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Create line shader if it doesn't exist
    if (lineShaderProgram == 0)
    {
        const char *lineVertexShaderSource = R"(
            #version 460 core
            layout (location = 0) in vec2 aPos;
            
            uniform mat4 uProjection;
            
            void main() {
                gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
            }
        )";

        const char *lineFragmentShaderSource = R"(
            #version 460 core
            out vec4 FragColor;
            
            uniform vec4 uColor;
            
            void main() {
                FragColor = uColor;
            }
        )";

        lineShaderProgram = createShaderProgram(lineVertexShaderSource, lineFragmentShaderSource);
    }

    // Set up orthographic projection
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width),
                                      static_cast<float>(height), 0.0f, -1.0f, 1.0f);

    // Use line shader
    glUseProgram(lineShaderProgram);

    // Set projection matrix
    GLint projLoc = glGetUniformLocation(lineShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Set line color (white with some transparency)
    GLint colorLoc = glGetUniformLocation(lineShaderProgram, "uColor");
    glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    // Get the boundaries from the regions
    const auto &regions = uiManager->getRegions();

    // Get current boundary positions from UI manager
    float verticalBoundaryX = uiManager->getVerticalBoundaryPosition() * width;
    float horizontalBoundaryY = uiManager->getHorizontalBoundaryPosition() * height;

    // Set up VAO/VBO for lines
    unsigned int lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Enable blending for transparent lines
    GLboolean blendEnabled;
    glGetBooleanv(GL_BLEND, &blendEnabled);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable line smoothing and set line width
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);

    // Draw vertical line
    float verticalLineVertices[] = {
        verticalBoundaryX, 0.0f,
        verticalBoundaryX, static_cast<float>(height)};

    glBufferData(GL_ARRAY_BUFFER, sizeof(verticalLineVertices), verticalLineVertices, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    // Draw horizontal line
    float horizontalLineVertices[] = {
        0.0f, horizontalBoundaryY,
        static_cast<float>(width), horizontalBoundaryY};

    glBufferData(GL_ARRAY_BUFFER, sizeof(horizontalLineVertices), horizontalLineVertices, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    // Clean up
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);

    // Restore line width
    glLineWidth(1.0f);

    // Disable line smoothing
    glDisable(GL_LINE_SMOOTH);

    // Restore blend state
    if (!blendEnabled)
    {
        glDisable(GL_BLEND);
    }

    // Restore viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Restore depth testing
    if (depthTestEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
}