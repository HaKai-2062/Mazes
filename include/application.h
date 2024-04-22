

class Application
{
public:
    Application() = delete;
    Application(Application&) = delete;
    Application(Application&&) = delete;


    Application(uint16_t* width, uint16_t* height)
        : m_Width(width), m_Height(height)
    {
        m_Maze = new Maze(*width, *height);
        m_Route = std::make_pair<uint32_t, uint32_t>(rand() % (m_Maze->m_MazeArea - 1), rand() % (m_Maze->m_MazeArea - 1));
    }

    ~Application()
    {
        DeleteMaze();
    }

    void GetButtonStates()
    {
        if (!m_Maze->MazeCompleted())
        {
            if (IsButtonPressed(BUILDER_RECURSIVE_BACKTRACK))
            {
                if (!m_MazeBuilder)
                {
                    m_BuilderSelected = MazeBuilder::Algorithms::RECURSIVE_BACKTRACK;
                    m_MazeBuilder = new MazeBuilder(m_Maze, static_cast<uint8_t>(m_BuilderSelected));
                }
                m_MazeBuilder->RecursiveBacktrack();
            }

            if (IsButtonPressed(BUILDER_KRUSKAL))
            {
                if (!m_MazeBuilder)
                {
                    m_BuilderSelected = MazeBuilder::Algorithms::KRUSKAL;
                    m_MazeBuilder = new MazeBuilder(m_Maze, static_cast<uint8_t>(m_BuilderSelected));
                }
                m_MazeBuilder->RandomizedKruskal();
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
            m_ButtonStates &= ~SOLVER_BFS;
            m_ButtonStates &= ~SOLVER_DFS;
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
        }

        // Always want to keep reset buttons pressable after maze completion
        m_ButtonStates &= ~MAZE;
        m_ButtonStates &= ~PATH;

        if (m_MazeBuilder && m_MazeBuilder->m_Completed && (IsButtonPressed(SOLVER_DFS) || IsButtonPressed(SOLVER_BFS)) && (!m_MazeSolver || !m_MazeSolver->m_Completed))
        {
            if (IsButtonPressed(SOLVER_DFS))
                m_SolverSelected = MazeSolver::DFS;
            if (IsButtonPressed(SOLVER_BFS))
                m_SolverSelected = MazeSolver::BFS;

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
                        m_MazeSolver->m_Completed = true;
                        std::cout << "Maze Solved. Goal is " << m_MazeSolver->m_Path.size() << " away!" << std::endl;
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
                        m_MazeSolver->m_Completed = true;
                        std::cout << "Maze Solved. Goal is " << m_MazeSolver->m_Path.size() << " away!" << std::endl;
                        m_MazeSolver->OnCompletion();
                    }
                    else
                    {
                        m_MazeSolver->BreadthFirstSearch();
                    }
                    break;
                }
            }
        }
    }

    uint32_t GetPathIfFound()
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

    bool IsButtonPressed(uint16_t buttonPressed) const
    {
        return (m_ButtonStates & buttonPressed) == 0 ? false : true;
    }

    void DeleteMaze()
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
    }

public:
    uint16_t* m_Width, *m_Height = nullptr;

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
        SOLVER_DFS = 0x10,
        SOLVER_BFS = 0x20
    };

    uint16_t m_ButtonStates = 0x00;

    MazeBuilder::Algorithms m_BuilderSelected = MazeBuilder::Algorithms::RECURSIVE_BACKTRACK;
    MazeSolver::Algorithms m_SolverSelected = MazeSolver::Algorithms::DFS;

    std::pair<uint32_t, uint32_t> m_Route;
};