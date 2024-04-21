#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <numeric>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "shader.h"
#include "maze.h"
#include "algorithms.h"
#include "application.h"
#include "imguiHandler.h"

void callbackResize(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
uint16_t SCR_WIDTH = 1280;
uint16_t SCR_HEIGHT = 720;

bool windowResized = false;


const char* mazeVertexShader = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor; \n"
"out vec4 ourColor;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.xy, 1.0, 1.0);\n"
"    ourColor = aColor;\n"
"}\n";

const char* mazeFragmentShader = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = ourColor;\n"
"}\n";


const char* pathVertexShader = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor; \n"
"out vec4 ourColor;\n"
"out vec2 ourPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.xy, 1.0, 1.0);\n"
"    ourColor = aColor;\n"
"    ourPos = aPos;\n"
"}\n";


const char* pathFragmentShader = "//Original Shader Source: https://www.shadertoy.com/view/lfXXzH\n"
"// Author @kyndinfo - 2016\n"
"// http://www.kynd.info\n"
"// Title: Marble\n"
""
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 ourColor;\n"
"in vec2 ourPos;\n"
"uniform bool overrideColor;\n"
"uniform float time;\n"
"float rand(vec2 co) {\n"
"    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);\n"
"}"
"float noise(vec2 p) {\n"
"    vec2 i = floor(p);  // Integer grid coordinatesn\n"
"    vec2 f = fract(p);  // Fractional part for interpolation\n"

"    // Smooth Hermite interpolation\n"
"    f = f * f * (3.0 - 2.0 * f);\n"

"    // Evaluate noise at each corner of the grid cell\n"
"    float a = rand(i);\n"
"    float b = rand(i + vec2(1.0, 0.0));\n"
"    float c = rand(i + vec2(0.0, 1.0));\n"
"    float d = rand(i + vec2(1.0, 1.0));\n"
""
"    // Bilinear interpolation\n"
"    vec2 u = f;\n"
"    return mix(mix(a, b, u.x),\n"
"        mix(c, d, u.x), u.y);\n"
"}\n"
""
"#define OCTAVES 4\n"
"float fbm(in vec2 pos) {\n"
"    // Initial values\n"
"    float value = 0.0;\n"
"    float amplitud = .5;\n"
"    // Loop of octaves\n"
"    for (int i = 0; i < OCTAVES; i++) {\n"
"        value += amplitud * noise(pos);\n"
"        pos *= 2.5;\n"
"        amplitud *= .6;\n"
"    }\n"
"    return value;\n"
"}\n"
""
"float edge(in vec2 v, float edge0, float edge1) {\n"
"    return 1.0 - smoothstep(edge0, edge1, abs(fbm(v) - 0.5));\n"
"}"
""
""
"void main()\n"
"{\n"
"   if (!overrideColor)\n"
"   {\n"
"       FragColor = ourColor;\n"
"       return;\n"
"   }\n"
""
"   vec3 redMeat = ourColor.xyz;\n"
"   vec3 fattyMeat = ourColor.xyz;\n"
""
"   // Animation - pulse using a sine function\n"
"   float pulse = 0.06 * sin(time * 1.5);  // Values between -0.01 and 0.01\n"
""
"   float v0 = edge(ourPos * 4.0, 0.0, 0.5); // Bass-note texture\n"
"   float v1 = smoothstep(0.6, 0.81, fbm(ourPos * 8.0)); // splotches\n"
"   float v2 = edge(ourPos * 6.0, 0.0, 0.2); // veins\n"
"   float v3 = edge(ourPos * 64.0, 0.0, 0.5); // high-note texture\n"
""
"   float gradientPos = 0.0;\n"
"   gradientPos += v0 * 0.55;\n"
"   gradientPos = mix(gradientPos, 0.45, v1);\n"
"   gradientPos = mix(gradientPos, 0.74 + pulse, v2);\n"
"   gradientPos += v3 * 0.27;\n"
""
"   vec3 col = redMeat * gradientPos;\n"
"   col = mix(col, fattyMeat, smoothstep(0.55, 1.0, gradientPos) - smoothstep(0.65, 1.0, gradientPos));\n"
"   col = mix(col, vec3(0.85), smoothstep(0.95, 1.0, gradientPos));\n"
""
"   FragColor = vec4(col, ourColor.w);\n"
"}\n";

int main()
{
    srand(clock());
    const char* glslVersion = "#version 130";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Maze", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // This disables V-sync
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, callbackResize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    ImGuiHandler::Init(glslVersion, window);

    // Create and bind a framebuffer object (FBO)
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a texture to render to
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer is not complete!" <<std::endl;

    Shader mazeShader(mazeVertexShader, mazeFragmentShader, true);
    Shader pathShader(pathVertexShader, pathFragmentShader, true);

    uint32_t VAO, VBO, EBO, VBOLine, EBOLine;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBOLine);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &EBOLine);
    glBindVertexArray(0);

    Application application(&SCR_WIDTH, &SCR_HEIGHT);
    
    static float f = 0.0f;
    static int counter = 0;
    bool showDemoWindow = false;

    ImVec2 getRegion = ImVec2(-1, -1);

    float deltaTime = 0.0f;
    float lastFrameTime = 0.0f;
    float localAccumulator = 0.0f;                  // This gets reset to 0 to keep 60fps
    float globalAccumulator = 0.0f;                 // This is the time uniform for shader
    const float shaderDelay = 1.0f / 60.0f;         // Shader updated at 60 FPS

    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        localAccumulator += deltaTime;

        processInput(window);

        ImGuiID dockSpaceID;
        ImGuiHandler::BeginFrame(dockSpaceID, showDemoWindow, application);

        // To store inside a framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        mazeShader.use();

        // Clear states of the Maze per frame
        // TDL: This is very inefficient and can be improved?
        application.m_Maze->m_Vertices.clear();
        application.m_Maze->m_Indices.clear();
        application.m_Maze->m_CellOrigin.clear();
        application.m_Maze->m_LineVertices.clear();
        application.m_Maze->m_LineIndices.clear();
        
        // This ensures that our initial range of route doesnt go out of maze range or they are not equal
        if (std::max(application.m_Route.first, application.m_Route.second) >= application.m_Maze->m_MazeArea ||
            application.m_Route.second == application.m_Route.first)
        {
            application.m_Route.first = (rand() % application.m_Maze->m_MazeArea - 1);
            application.m_Route.second = (rand() % application.m_Maze->m_MazeArea - 1);
        }

        application.GetButtonStates();

        uint32_t rectangleCount = 0;
        if (!application.m_MazeBuilder)
            rectangleCount = application.m_Maze->DrawMaze();
        else
            rectangleCount = application.m_Maze->DrawMaze(&application.m_MazeBuilder->m_Stack);

        uint32_t rectangleCount2 = 0;
        rectangleCount2 = application.GetPathIfFound();

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, application.m_Maze->m_Vertices.size() * sizeof(float) * 2, application.m_Maze->m_Vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, application.m_Maze->m_Indices.size() * sizeof(uint32_t), application.m_Maze->m_Indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawElements(GL_TRIANGLES, rectangleCount * 6, GL_UNSIGNED_INT, 0);

        pathShader.use();

        if (application.m_ColorPathAnimation1)
            pathShader.setBool("overrideColor", true);
        else
            pathShader.setBool("overrideColor", false);

        if (localAccumulator > shaderDelay)
        {
            globalAccumulator += localAccumulator;
            localAccumulator = 0.0f;
            pathShader.setFloat("time", globalAccumulator);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, VBOLine);
        glBufferData(GL_ARRAY_BUFFER, application.m_Maze->m_LineVertices.size() * sizeof(float) * 2, application.m_Maze->m_LineVertices.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOLine);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, application.m_Maze->m_LineIndices.size() * sizeof(uint32_t), application.m_Maze->m_LineIndices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawElements(GL_TRIANGLES, rectangleCount2 * 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwPollEvents();

        // For Controls menu
        ImGui::End();

        ImGuiHandler::EndFrame(dockSpaceID, &texture, getRegion, windowResized);

        // Delete old framebuffer and create a new one
        if (windowResized)
        {
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteTextures(1, &texture);

            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

            // Create a texture to render to
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getRegion.x, getRegion.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

            callbackResize(window, getRegion.x, getRegion.y);

            // Rebuild the maze if we have not yet started maze generation
            if (!application.m_MazeBuilder)
            {
                delete application.m_Maze;
                application.m_Maze = new Maze(SCR_WIDTH, SCR_HEIGHT);
            }

            windowResized = false;
        }

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);

        std::this_thread::sleep_for(std::chrono::milliseconds(application.m_Delay));
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void callbackResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    windowResized = true;
}