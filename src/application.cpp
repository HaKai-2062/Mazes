#include <iostream>

#include "application.h"

Application::Application(uint16_t * width, uint16_t * height)
    : m_Width(width), m_Height(height)
{
    m_Maze = new Maze(*width, *height);
    m_Route = std::make_pair<uint32_t, uint32_t>(rand() % (m_Maze->m_MazeArea - 1), rand() % (m_Maze->m_MazeArea - 1));
}

Application::~Application()
{
    DeleteMaze();
}

void Application::GetButtonStates()
{
    if (!m_Maze)
        return;

    if (!m_Maze->MazeCompleted())
    {
        // To get the offset
        uint16_t j = BUILDER_RECURSIVE_BACKTRACK;

        for (uint32_t i = MazeBuilder::Algorithms::RECURSIVE_BACKTRACK; i <= MazeBuilder::Algorithms::WILSON; i++, j*=2)
        {
            if (IsButtonPressed(j) && !m_MazeBuilder)
            {
                m_BuilderSelected = static_cast<MazeBuilder::Algorithms>(i);
                m_MazeBuilder = new MazeBuilder(m_Maze, i);
                break;
            }
        }

        switch (m_BuilderSelected)
        {
        case MazeBuilder::Algorithms::RECURSIVE_BACKTRACK:
        {
            m_MazeBuilder->RecursiveBacktrack();
            break;
        }
        case MazeBuilder::Algorithms::KRUSKAL:
        {
            m_MazeBuilder->RandomizedKruskal();
            break;
        }
        case MazeBuilder::Algorithms::PRIMS:
        {
            m_MazeBuilder->RandomizedPrims();
            break;
        }
        case MazeBuilder::Algorithms::WILSON:
        {
            m_MazeBuilder->Wilson();
            break;
        }
        }
    }
    else if (m_MazeBuilder && !m_MazeBuilder->m_Completed)
    {
        std::cout << "Maze Generated\n";
        m_MazeBuilder->m_Completed = true;
        m_MazeBuilder->OnCompletion();

        m_ButtonStates &= ~SOLVER_BFS;
        m_ButtonStates &= ~SOLVER_DFS;
    }

    if (IsButtonPressed(MAZE))
    {
        DeleteMaze();
        m_Maze = new Maze(*m_Width, *m_Height);

        m_ButtonStates &= ~BUILDER_RECURSIVE_BACKTRACK;
        m_ButtonStates &= ~BUILDER_KRUSKAL;
        m_ButtonStates &= ~BUILDER_PRIMS;
        m_ButtonStates &= ~BUILDER_WILSON;
        m_ButtonStates &= ~SOLVER_BFS;
        m_ButtonStates &= ~SOLVER_DFS;
        m_ButtonStates &= ~SOLVER_DIJKSTRA;
        m_ButtonStates &= ~SOLVER_ASTAR;
    }

    if (IsButtonPressed(PATH) && m_MazeSolver && m_MazeSolver->m_Completed)
    {
        delete m_MazeSolver;

        m_MazeSolver = nullptr;

        for (uint32_t i = 0; m_Maze && i < m_Maze->m_VisitedCellInfo.size(); i++)
        {
            m_Maze->m_VisitedCellInfo[i] &= ~Maze::CELL_SEARCHED;
        }

        m_ButtonStates &= ~SOLVER_BFS;
        m_ButtonStates &= ~SOLVER_DFS;
        m_ButtonStates &= ~SOLVER_DIJKSTRA;
        m_ButtonStates &= ~SOLVER_ASTAR;
    }

    // Always want to keep reset buttons pressable after maze completion
    m_ButtonStates &= ~MAZE;
    m_ButtonStates &= ~PATH;

    if (m_MazeBuilder && m_MazeBuilder->m_Completed && 
        (IsButtonPressed(SOLVER_DFS) || IsButtonPressed(SOLVER_BFS) || IsButtonPressed(SOLVER_DIJKSTRA) || IsButtonPressed(SOLVER_ASTAR)) &&
        (!m_MazeSolver || !m_MazeSolver->m_Completed))
    {
        if (IsButtonPressed(SOLVER_DFS))
            m_SolverSelected = MazeSolver::DFS;
        if (IsButtonPressed(SOLVER_BFS))
            m_SolverSelected = MazeSolver::BFS;
        if (IsButtonPressed(SOLVER_DIJKSTRA))
            m_SolverSelected = MazeSolver::DIJKSTRA;
        if (IsButtonPressed(SOLVER_ASTAR))
            m_SolverSelected = MazeSolver::ASTAR;

        if (!m_MazeSolver)
        {
            m_MazeSolver = new MazeSolver(m_Maze, static_cast<uint8_t>(m_SolverSelected), m_Route);
            std::cout << m_Route.first << ',' << m_Route.second << std::endl;
        }

        switch (m_SolverSelected)
        {
        case MazeSolver::DFS:
        {
            if (!m_MazeSolver->m_Stack.empty() && m_MazeSolver->m_Stack.top() == m_Route.second)
            {
                m_MazeSolver->OnCompletion();
            }
            else
            {
                m_MazeSolver->DepthFirstSearch();
            }
            break;
        }
        case MazeSolver::BFS:
        {
            if (!m_MazeSolver->m_Queue.empty() && m_MazeSolver->m_Queue.front() == m_Route.second)
            {
                m_MazeSolver->OnCompletion();
            }
            else
            {
                m_MazeSolver->BreadthFirstSearch();
            }
            break;
        }
        case MazeSolver::DIJKSTRA:
        {
            // Waiting for queue to be empty guarentees shortest path
            //if (m_MazeSolver->m_PQueue.empty())
            if (!m_MazeSolver->m_PQueue.empty() && m_MazeSolver->m_PQueue.top().id == m_Route.second)
            {
                m_MazeSolver->OnCompletion();
            }
            else
            {
                m_MazeSolver->DijkstraSearch();
            }
            break;
        }
        case MazeSolver::ASTAR:
        {
            // Waiting for queue to be empty guarentees shortest path
            //if (m_MazeSolver->m_PQueue.empty())
            if (!m_MazeSolver->m_PQueue.empty() && m_MazeSolver->m_PQueue.top().id == m_Route.second)
            {
                m_MazeSolver->OnCompletion();
            }
            else
            {
                m_MazeSolver->AstarSearch();
            }
            break;
        }
        }
    }
}

uint32_t Application::GetPathIfFound()
{
    if (!m_MazeSolver || !m_MazeSolver->m_Completed)
        return 0;

    uint32_t elementsToDraw = 0;
    auto colorOfLineVertex = [&]()
        {
            m_Maze->m_LineVertices.push_back(std::make_pair(m_ColorPath[0], m_ColorPath[1]));
            m_Maze->m_LineVertices.push_back(std::make_pair(m_ColorPath[2], m_ColorPath[3]));
        };

    std::vector<uint32_t> path = m_MazeSolver->m_Path;

    while (!path.empty())
    {
        uint32_t firstCell = path.back();
        path.pop_back();

        if (path.empty())
            break;

        uint32_t secondCell = path.back();
        path.pop_back();
        path.push_back(secondCell);

        // Adjust spacing
        bool normalDrawing = false;

        // Swap them and always draw towards East and South
        if (secondCell < firstCell)
        {
            uint32_t temp = secondCell;
            secondCell = firstCell;
            firstCell = temp;
            normalDrawing = true;
        }

        bool isSouth = secondCell - firstCell == 1 ? true : false;
        float normalizedHalfCellWidth = static_cast<float>(2 * m_Maze->m_HalfCellHeight) / (m_Maze->m_MazeWidth);
        float normalizedHalfCellHeight = static_cast<float>(2 * m_Maze->m_HalfCellHeight) / (m_Maze->m_MazeHeight);
        float normalizedTotalCellWidth = static_cast<float>(2 * m_Maze->m_TotalCellHeight) / (m_Maze->m_MazeWidth);
        float normalizedTotalCellHeight = static_cast<float>(2 * m_Maze->m_TotalCellHeight) / (m_Maze->m_MazeHeight);
        float normalizedLineThickness = static_cast<float>(2 * m_Maze->m_LineThickness) / (m_Maze->m_MazeHeight);

        std::pair<float, float> firstPoint = m_Maze->m_CellOrigin[firstCell];
        std::pair<float, float> secondPoint = m_Maze->m_CellOrigin[secondCell];

        if (isSouth)
        {
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 0);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 1);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 3);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 0);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 2);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 3);

            // top left
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(firstPoint.first - normalizedLineThickness, firstPoint.second + (normalDrawing ? 0 : -normalizedLineThickness)));
            colorOfLineVertex();
            // top right
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(firstPoint.first + normalizedLineThickness, firstPoint.second + (normalDrawing ? 0 : -normalizedLineThickness)));
            colorOfLineVertex();
            // bottom left
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(secondPoint.first - normalizedLineThickness, secondPoint.second + (normalDrawing ? normalizedLineThickness : 0)));
            colorOfLineVertex();
            // bottom right
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(secondPoint.first + normalizedLineThickness, secondPoint.second + (normalDrawing ? normalizedLineThickness : 0)));
            colorOfLineVertex();
        }
        else
        {
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 0);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 1);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 3);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 0);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 2);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 3);

            // left top
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(firstPoint.first + (normalDrawing ? 0 : -normalizedLineThickness), firstPoint.second + normalizedLineThickness));
            colorOfLineVertex();
            // left bottom
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(firstPoint.first + (normalDrawing ? 0 : -normalizedLineThickness), firstPoint.second - normalizedLineThickness));
            colorOfLineVertex();
            // right top
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(secondPoint.first + (normalDrawing ? normalizedLineThickness : 0), secondPoint.second + normalizedLineThickness));
            colorOfLineVertex();
            // right bottom
            m_Maze->m_LineVertices.push_back(std::make_pair<float, float>(secondPoint.first + (normalDrawing ? normalizedLineThickness : 0), secondPoint.second - normalizedLineThickness));
            colorOfLineVertex();
        }
        elementsToDraw++;
    }
    return elementsToDraw;
}

bool Application::IsButtonPressed(uint16_t buttonPressed) const
{
    return (m_ButtonStates & buttonPressed) == 0 ? false : true;
}

void Application::DeleteMaze()
{
    if (m_Maze)
        delete m_Maze;
    if (m_MazeBuilder)
        delete m_MazeBuilder;
    if (m_MazeSolver)
        delete m_MazeSolver;

    m_Maze = nullptr;
    m_MazeBuilder = nullptr;
    m_MazeSolver = nullptr;
    m_BuilderSelected = MazeBuilder::Algorithms::NONE;
    m_SolverSelected = MazeSolver::Algorithms::NONE;
}