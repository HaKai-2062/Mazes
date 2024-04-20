

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
    }

    ~Application()
    {
        if (m_Maze)
            delete m_Maze;    
        if (m_MazeBuilder)
            delete m_MazeBuilder;
        if (m_MazeSolver)
            delete m_MazeSolver;
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

            m_ButtonStates &= ~RESET;
            m_ButtonStates &= ~SOLVER_BFS;
            m_ButtonStates &= ~SOLVER_DFS;
        }

        if (IsButtonPressed(RESET))
        {
            if (m_Maze)
                delete m_Maze;
            m_Maze = new Maze(*m_Width, *m_Height);

            if (m_MazeBuilder)
                delete m_MazeBuilder;
            m_MazeBuilder = nullptr;

            if (m_MazeSolver)
                delete m_MazeSolver;
            m_MazeSolver = nullptr;

            m_ButtonStates &= ~BUILDER_RECURSIVE_BACKTRACK;
            m_ButtonStates &= ~BUILDER_KRUSKAL;
            m_ButtonStates &= ~SOLVER_BFS;
            m_ButtonStates &= ~SOLVER_DFS;
        }

        // Always want to keep reset button pressable after maze completion
        m_ButtonStates &= ~RESET;

        if (m_MazeBuilder && m_MazeBuilder->m_Completed && (IsButtonPressed(SOLVER_DFS) || IsButtonPressed(SOLVER_BFS)) && (!m_MazeSolver || !m_MazeSolver->m_Completed))
        {
            if (IsButtonPressed(SOLVER_DFS))
                m_SolverSelected = MazeSolver::DFS;
            if (IsButtonPressed(SOLVER_BFS))
                m_SolverSelected = MazeSolver::BFS;

            if (!m_MazeSolver)
            {
                m_MazeSolver = new MazeSolver(m_Maze, static_cast<uint8_t>(m_SolverSelected), route);
                std::cout << route.first << ',' << route.second << std::endl;
            }

            switch (m_SolverSelected)
            {
            case MazeSolver::DFS:
                if (!m_MazeSolver->m_Stack.empty() && m_MazeSolver->m_Stack.top() == route.second)
                {
                    m_MazeSolver->m_Completed = true;
                    std::cout << "Maze Solved. Goal is " << m_MazeSolver->m_Queue.size() << " away!" << std::endl;
                    m_MazeSolver->OnCompletion();
                }
                else
                {
                    m_MazeSolver->DepthFirstSearch();
                }
                break;
            case MazeSolver::BFS:
                if (!m_MazeSolver->m_Queue.empty() && m_MazeSolver->m_Queue.front() == route.second)
                {
                    m_MazeSolver->m_Completed = true;
                    std::cout << "Maze Solved. Goal is " << m_MazeSolver->m_Queue.size() << " away!" << std::endl;
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

    bool IsButtonPressed(uint16_t buttonPressed) const
    {
        return (m_ButtonStates & buttonPressed) == 0 ? false : true;
    }

public:
    uint16_t* m_Width, *m_Height = nullptr;

	Maze* m_Maze = nullptr;
    MazeBuilder* m_MazeBuilder = nullptr;
    MazeSolver* m_MazeSolver = nullptr;
	int m_Delay = 10;
    
    const enum
    {
        RESET = 0x01,
        BUILDER_RECURSIVE_BACKTRACK = 0x02,
        BUILDER_KRUSKAL = 0x04,
        SOLVER_DFS = 0x08,
        SOLVER_BFS = 0x10
    };

    uint16_t m_ButtonStates = 0x00;

    MazeBuilder::Algorithms m_BuilderSelected = MazeBuilder::Algorithms::RECURSIVE_BACKTRACK;
    MazeSolver::Algorithms m_SolverSelected = MazeSolver::Algorithms::DFS;

    std::pair<uint32_t, uint32_t> route = std::make_pair<uint32_t, uint32_t>(0, 800);
};