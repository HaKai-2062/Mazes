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

    struct Rect
    {
        float x1, y1, x2, y2, x3, y3, x4, y4;
    };
    Rect line{};

    uint32_t elementsToDraw = 0;
    auto colorAndNormalization = [&]()
        {
            // Color
            m_Maze->m_LineVertices.push_back(std::make_pair(m_ColorPath[0], m_ColorPath[1]));
            m_Maze->m_LineVertices.push_back(std::make_pair(m_ColorPath[2], m_ColorPath[3]));

            // Left bottom vertix to normalize every pixel from it
            // Additionally, Width and heights of line are pushed
            m_Maze->m_LineVertices.push_back({ line.x3, line.y3 });
            m_Maze->m_LineVertices.push_back({std::abs(line.x2 - line.x1), std::abs(line.y1 - line.y3)});
        };

    std::vector<uint32_t> path = m_MazeSolver->m_Path;
    // This is first element to indicate start of line
    path.push_back(m_Maze->m_MazeArea);

    float aspectRatioX = static_cast<float>(*m_Height) / *m_Width;
    float aspectRatioY = static_cast<float>(*m_Width) / *m_Height;

    if (aspectRatioX > 1)
        aspectRatioX = 1;
    if (aspectRatioY > 1)
        aspectRatioY = 1;

    // We are multiplying by 2 because the coordinate go from -1 to 1 instead of 0 to 1
    // Easier to think as dividing by 2 in denominator
    float normalizedHalfCellWidth = static_cast<float>(2 * m_Maze->m_HalfCellHeight) / (m_Maze->m_MazeWidth);
    float normalizedHalfCellHeight = static_cast<float>(2 * m_Maze->m_HalfCellHeight) / (m_Maze->m_MazeHeight);
    float normalizedWallThickness = static_cast<float>(2 * m_Maze->m_WallThickness) / (m_Maze->m_MazeHeight);

    float normalizedTotalCellWidth = static_cast<float>(2 * m_Maze->m_TotalCellHeight) / (m_Maze->m_MazeWidth);
    float normalizedTotalCellHeight = static_cast<float>(2 * m_Maze->m_TotalCellHeight) / (m_Maze->m_MazeHeight);

    float normalizedHalfLineThickness = static_cast<float>(m_Maze->m_LineThickness) / (m_Maze->m_MazeHeight);
    float normalizedLineThickness = 2 * normalizedHalfLineThickness;

    enum Directions
    {
        NONE = 0,
        EAST,
        WEST,
        NORTH,
        SOUTH
    };

    Directions direction = NONE;

    while (!path.empty())
    {
        // This is the previous cell from the current drawing one
        uint32_t zeroCell = path.back();
        path.pop_back();

        if (path.empty())
            break;
        uint32_t firstCell = path.back();
        path.pop_back();

        if (path.empty())
            break;
        uint32_t secondCell = path.back();
        path.pop_back();

        // Initialized to be max value
        uint32_t thirdCell = m_Maze->m_MazeArea;
        if (!path.empty())
        {
            thirdCell = path.back();
            path.pop_back();
        }

        if (thirdCell != m_Maze->m_MazeArea)
            path.push_back(thirdCell);
        path.push_back(secondCell);
        path.push_back(firstCell);

        // Go from cell1 to cell2
        auto getDirectionToDraw = [&](uint32_t cell1, uint32_t cell2)
            {
                if (cell1 == m_Maze->m_MazeArea || cell2 == m_Maze->m_MazeArea)
                    return NONE;

                if (cell2 > cell1)
                {
                    if (cell2 - cell1 == 1)
                        return SOUTH;
                    else
                        return EAST;
                }
                else
                {
                    if (cell1 - cell2 == 1)
                        return NORTH;
                    else
                        return WEST;
                }
            };

        Directions previousDir = getDirectionToDraw(zeroCell, firstCell);
        Directions mainDir = getDirectionToDraw(firstCell, secondCell);
        Directions nextDir = getDirectionToDraw(secondCell, thirdCell);

        std::pair<float, float> firstPoint = m_Maze->m_CellOrigin[firstCell];
        std::pair<float, float> secondPoint = m_Maze->m_CellOrigin[secondCell];

        if (mainDir == NORTH)
        {
            // bottom left
            line.x1 = secondPoint.first - (normalizedHalfLineThickness) * aspectRatioX;
            line.y1 = secondPoint.second + (normalizedHalfLineThickness - (nextDir == EAST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
            // bottom right
            line.x2 = secondPoint.first + (normalizedHalfLineThickness) * aspectRatioX;
            line.y2 = secondPoint.second + (normalizedHalfLineThickness - (nextDir == WEST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
            // top left
            line.x3 = firstPoint.first - (normalizedHalfLineThickness) * aspectRatioX;
            line.y3 = firstPoint.second + (normalizedHalfLineThickness - (previousDir == EAST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
            // top right
            line.x4 = firstPoint.first + (normalizedHalfLineThickness) * aspectRatioX;
            line.y4 = firstPoint.second + (normalizedHalfLineThickness - (previousDir == WEST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
        }
        else if (mainDir == SOUTH)
        {
            // top left
            line.x1 = firstPoint.first - (normalizedHalfLineThickness)*aspectRatioX;
            line.y1 = firstPoint.second + (normalizedHalfLineThickness - (previousDir == WEST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
            // top right
            line.x2 = firstPoint.first + (normalizedHalfLineThickness)*aspectRatioX;
            line.y2 = firstPoint.second + (normalizedHalfLineThickness - (previousDir == EAST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
            // bottom left
            line.x3 = secondPoint.first - (normalizedHalfLineThickness)*aspectRatioX;
            line.y3 = secondPoint.second + (normalizedHalfLineThickness - (nextDir == WEST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
            // bottom right
            line.x4 = secondPoint.first + (normalizedHalfLineThickness)*aspectRatioX;
            line.y4 = secondPoint.second + (normalizedHalfLineThickness - (nextDir == EAST ? normalizedLineThickness : 0.0f)) * aspectRatioY;
        }
        else if (mainDir == EAST)
        {
            // top left
            line.x1 = firstPoint.first + (normalizedHalfLineThickness - (previousDir == SOUTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y1 = firstPoint.second + (normalizedHalfLineThickness) * aspectRatioY;
            // top right
            line.x2 = secondPoint.first + (normalizedHalfLineThickness - (nextDir == SOUTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y2 = secondPoint.second + (normalizedHalfLineThickness) * aspectRatioY;
            // bottom left
            line.x3 = firstPoint.first + (normalizedHalfLineThickness - (previousDir == NORTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y3 = firstPoint.second - (normalizedHalfLineThickness) * aspectRatioY;
            // bottom right
            line.x4 = secondPoint.first + (normalizedHalfLineThickness - (nextDir == NORTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y4 = secondPoint.second - (normalizedHalfLineThickness) * aspectRatioY;
        }
        else if (mainDir == WEST)
        {
            // bottom left
            line.x1 = secondPoint.first + (normalizedHalfLineThickness - (nextDir == NORTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y1 = secondPoint.second + (normalizedHalfLineThickness) * aspectRatioY;
            // bottom right
            line.x2 = firstPoint.first + (normalizedHalfLineThickness - (previousDir == NORTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y2 = firstPoint.second + (normalizedHalfLineThickness) * aspectRatioY;
            // top left
            line.x3 = secondPoint.first + (normalizedHalfLineThickness - (nextDir == SOUTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y3 = secondPoint.second - (normalizedHalfLineThickness) * aspectRatioY;
            // top right
            line.x4 = firstPoint.first + (normalizedHalfLineThickness - (previousDir == SOUTH ? normalizedLineThickness : 0.0f)) * aspectRatioX;
            line.y4 = firstPoint.second - (normalizedHalfLineThickness) * aspectRatioY;
        }

        if (mainDir != NONE)
        {
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 0);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 1);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 3);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 0);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 2);
            m_Maze->m_LineIndices.push_back((4 * elementsToDraw) + 3);

            m_Maze->m_LineVertices.push_back({ line.x1, line.y1 });
            colorAndNormalization();
            m_Maze->m_LineVertices.push_back({ line.x2, line.y2 });
            colorAndNormalization();
            m_Maze->m_LineVertices.push_back({ line.x3, line.y3 });
            colorAndNormalization();
            m_Maze->m_LineVertices.push_back({ line.x4, line.y4 });
            colorAndNormalization();
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