#include <stdint.h>

#include "maze.h"
#include "mazeBuilder.h"
#include "mazeSolver.h"

class Application
{
public:
    Application() = delete;
    Application(Application&) = delete;
    Application(Application&&) = delete;
    Application(uint16_t* width, uint16_t* height);
    ~Application();

    void GetButtonStates();
    uint32_t GetPathIfFound();
    bool IsButtonPressed(uint16_t buttonPressed) const;
    void DeleteMaze();

public:
    uint16_t* m_Width, * m_Height = nullptr;

    Maze* m_Maze = nullptr;
    MazeBuilder* m_MazeBuilder = nullptr;
    MazeSolver* m_MazeSolver = nullptr;
    int m_Delay = 10;

    bool m_PathAnimation = true;
    float m_PathSpeed = 0.05;
    float m_ColorPath[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

    const enum
    {
        PATH = 0x01,
        MAZE = 0x02,
        BUILDER_RECURSIVE_BACKTRACK = 0x04,
        BUILDER_KRUSKAL = 0x08,
        BUILDER_PRIMS = 0x10,
        BUILDER_WILSON = 0x20,
        SOLVER_DFS = 0x40,
        SOLVER_BFS = 0x80,
        SOLVER_DIJKSTRA = 0x100,
        SOLVER_ASTAR = 0x200
    };

    uint16_t m_ButtonStates = 0x00;
    
    MazeBuilder::Algorithms m_BuilderSelected = MazeBuilder::Algorithms::NONE;
    MazeSolver::Algorithms m_SolverSelected = MazeSolver::Algorithms::NONE;

    std::pair<uint32_t, uint32_t> m_Route;
};