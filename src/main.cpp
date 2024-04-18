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
#include "imguiHandler.h"

void callbackResize(GLFWwindow* window, int width, int height);
void getButtonStates(Maze*& mazeObject, MazeBuilder*& mazeBuilder, MazeBuilder::Algorithms& builderSelected, bool& builderButton1, bool& builderButton2, bool& resetButton);
void processInput(GLFWwindow* window);

// settings
uint16_t SCR_WIDTH = 1280;
uint16_t SCR_HEIGHT = 720;

bool windowResized = false;


const char* vertexShader = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor; \n"
"out vec4 ourColor;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.xy, 1.0, 1.0);\n"
"    ourColor = aColor;\n"
"}\n";

const char* fragmentShader = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec4 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = ourColor;\n"
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

    Shader backgroundShader(vertexShader, fragmentShader, true);

    uint32_t VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(0);

    std::vector<std::pair<float, float>> vertices;
    std::vector<uint32_t> indices;

    Maze* mazeObject = new Maze(SCR_WIDTH, SCR_HEIGHT);
    
    static float f = 0.0f;
    static int counter = 0;
    bool showDemoWindow = false;
    int delay = 10;

    bool builderButton1 = false;
    bool builderButton2 = false;
    bool resetButton = false;

    MazeBuilder::Algorithms builderSelected;
    MazeBuilder* mazeBuilder = nullptr;
    // Selected at random
    //std::pair<uint32_t, uint32_t> route = std::make_pair<uint32_t, uint32_t>(1,2);

    ImVec2 getRegion = ImVec2(-1, -1);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        ImGuiID dockSpaceID;
        ImGuiHandler::BeginFrame(dockSpaceID, showDemoWindow, delay, mazeBuilder, builderButton1, builderButton2, resetButton);

        // To store inside a framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        backgroundShader.use();

        // Clear states of the Maze per frame
        // TDL: This is very inefficient and can be improved?
        vertices.clear();
        indices.clear();
        
        getButtonStates(mazeObject, mazeBuilder, builderSelected, builderButton1, builderButton2, resetButton);

        uint32_t rectangleCount = 0;
        if (!mazeBuilder)
            rectangleCount = mazeObject->DrawMaze(vertices, indices);
        else
            rectangleCount = mazeObject->DrawMaze(vertices, indices, &mazeBuilder->m_Stack);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 2, vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glDrawElements(GL_TRIANGLES, rectangleCount * 6, GL_UNSIGNED_INT, 0);
        
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
            if (!mazeBuilder)
            {
                delete mazeObject;
                mazeObject = new Maze(SCR_WIDTH, SCR_HEIGHT);
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

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
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

void getButtonStates(Maze*& mazeObject, MazeBuilder*& mazeBuilder, MazeBuilder::Algorithms& builderSelected, bool& builderButton1, bool& builderButton2, bool& resetButton)
{
    if (!mazeObject->MazeCompleted())
    {
        if (builderButton1)
        {
            builderSelected = MazeBuilder::Algorithms::RECURSIVE_BACKTRACK;
            if (!mazeBuilder)
                mazeBuilder = new MazeBuilder(mazeObject, static_cast<uint8_t>(builderSelected));
            mazeBuilder->RecursiveBacktrack();
        }

        if (builderButton2)
        {
            builderSelected = MazeBuilder::Algorithms::KRUSKAL;
            if (!mazeBuilder)
                mazeBuilder = new MazeBuilder(mazeObject, static_cast<uint8_t>(builderSelected));
            mazeBuilder->RandomizedKruskal();
        }
    }
    else if (mazeBuilder && !mazeBuilder->m_Completed)
    {
        std::cout << "Maze Generated\n";
        mazeBuilder->m_Completed = true;
        mazeBuilder->OnCompletion();
        builderButton1 = true;
        builderButton2 = true;
    }

    if (resetButton)
    {
        delete mazeBuilder;
        mazeBuilder = nullptr;

        delete mazeObject;
        mazeObject = new Maze(SCR_WIDTH, SCR_HEIGHT);

        builderButton1 = false;
        builderButton2 = false;
    }

    // Always want to keep reset button pressable after maze completion
    resetButton = false;
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