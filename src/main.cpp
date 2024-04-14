#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "maze.h"
#include "algorithms.h"

void callbackResize(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const uint16_t SCR_WIDTH = 1000;
const uint16_t SCR_HEIGHT = 1000;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    srand(clock());

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
    glfwSetFramebufferSizeCallback(window, callbackResize);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader backgroundShader("resources\\rect.vs", "resources\\rect.ps");

    uint32_t VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(0);

    std::vector<std::pair<float, float>> vertices;
    std::vector<uint32_t> indices;

    Maze mazeObject(SCR_WIDTH, SCR_HEIGHT);
    float totalCellWidth = mazeObject.m_HalfCellHeight * 2 + mazeObject.m_WallThickness;
    uint16_t cellsInOneAxis = static_cast<uint16_t>(2 / totalCellWidth);
    uint32_t startCoordinate = rand() % static_cast<uint32_t>(cellsInOneAxis * cellsInOneAxis - 1);
    std::stack<uint32_t> stack;
    stack.push(startCoordinate);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        backgroundShader.use();

        // Clear states of the Maze per frame
        // TDL: This is very inefficient and can be improved
        vertices.clear();
        indices.clear();
        mazeObject.ClearMazeVariables();

        if (!mazeObject.MazeCompleted())
        {
            MazeBuilder::RecursiveBacktrack(mazeObject.m_VisitedCellInfo, stack, totalCellWidth, mazeObject.m_VisitedCellCount);
        }
        else
        {
            // std::cout << "Completed\n";
            // Solve the maze here if needed
        }

        uint32_t rectangleCount = mazeObject.DrawMaze(vertices, indices, stack.top());

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

        glfwSwapBuffers(window);
        glfwPollEvents();

        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
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
}