#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
// application.h is pre-included so only include this once
#include "imguiHandler.h"

void callbackResize(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, double& mouseX, double& mouseY, ImVec2& vMin, ImVec2& vMax, Application& application);

// settings
uint16_t SCR_WIDTH = 1280;
uint16_t SCR_HEIGHT = 720;

bool windowResized = false;

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    // mazeVertexShader, mazeFragmentShader, pathVertexShader, pathFragmentShader are defined in them
    #include "../../res/shaders/maze.vs"
    #include "../../res/shaders/maze.ps"
    #include "../../res/shaders/path.vs"
    #include "../../res/shaders/path.ps"

    Shader mazeShader(mazeVertexShader.c_str(), mazeFragmentShader.c_str(), true);
    Shader pathShader(pathVertexShader.c_str(), pathFragmentShader.c_str(), true);

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

        double mouseX = 0.0, mouseY = 0.0;

        ImGuiID dockSpaceID;
        ImGuiHandler::BeginFrame(dockSpaceID, showDemoWindow, application);

        // To store inside a framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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
            rectangleCount = application.m_Maze->DrawMaze(&application.m_MazeBuilder->m_Path, &application.m_Route);

        uint32_t rectangleCount2 = 0;
        rectangleCount2 = application.GetPathIfFound();

        mazeShader.use();

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, application.m_Maze->m_Vertices.size() * sizeof(float) * 2, application.m_Maze->m_Vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, application.m_Maze->m_Indices.size() * sizeof(uint32_t), application.m_Maze->m_Indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 6 values in index buffer per cell
        glDrawElements(GL_TRIANGLES, rectangleCount * 6, GL_UNSIGNED_INT, 0);

        pathShader.use();

        if (application.m_PathAnimation)
            pathShader.setBool("enableAnimation", true);
        else
            pathShader.setBool("enableAnimation", false);

        pathShader.setFloat("colorCycle", application.m_PathSpeed);

        if (localAccumulator > shaderDelay)
        {
            globalAccumulator += localAccumulator;
            localAccumulator = 0.0f;
            pathShader.setFloat("time", globalAccumulator);
        }

        glBindBuffer(GL_ARRAY_BUFFER, VBOLine);
        glBufferData(GL_ARRAY_BUFFER, application.m_Maze->m_LineVertices.size() * sizeof(float) * 2, application.m_Maze->m_LineVertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOLine);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, application.m_Maze->m_LineIndices.size() * sizeof(uint32_t), application.m_Maze->m_LineIndices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(2 * sizeof(float))); 
        glEnableVertexAttribArray(1);

        // Reference point
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // Width, Height
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // 6 values in index buffer per cell
        glDrawElements(GL_TRIANGLES, rectangleCount2 * 6, GL_UNSIGNED_INT, 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glfwPollEvents();

        // For Controls menu
        ImGui::End();

        ImGuiHandler::EndFrame(dockSpaceID, &texture, getRegion, windowResized);

        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;

        //ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));

        processInput(window, mouseX, mouseY, vMin, vMax, application);

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, double& mouseX, double& mouseY, ImVec2& vMin, ImVec2& vMax, Application& application)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) 
        && ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) || (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)))
    {
        glfwGetCursorPos(window, &mouseX, &mouseY);
        int windowX = 0, windowY = 0;
        glfwGetWindowPos(window, &windowX, &windowY);

        // These x,y are from 0,0 top left to 1,1 bottom right
        double xPos = (mouseX - vMin.x + windowX) / (vMax.x - vMin.x);
        double yPos = (mouseY - vMin.y + windowY) / (vMax.y - vMin.y);

        float normalizedTotalCellWidth = static_cast<float>(application.m_Maze->m_TotalCellHeight) / (application.m_Maze->m_MazeWidth);
        float normalizedTotalCellHeight = static_cast<float>(application.m_Maze->m_TotalCellHeight) / (application.m_Maze->m_MazeHeight);

        int cellInX = static_cast<int>(xPos / normalizedTotalCellWidth);
        int cellInY = static_cast<int>(yPos / normalizedTotalCellHeight);

        uint32_t cellNumber = cellInX * application.m_Maze->m_CellsAcrossHeight + cellInY;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            application.m_Route.first = cellNumber;
        }
        else
        {
            application.m_Route.second = cellNumber;
        }
    }
}

void callbackResize(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    windowResized = true;
}