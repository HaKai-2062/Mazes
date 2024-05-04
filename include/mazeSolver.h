#include <queue>
#include <stack>

class Maze;

class MazeSolver
{
public:
	MazeSolver() = delete;
	MazeSolver(MazeSolver&) = delete;
	MazeSolver(MazeSolver&&) = delete;

	MazeSolver(Maze* maze, uint8_t selectedAlgorithm, std::pair<uint32_t, uint32_t>& route);
	~MazeSolver();
	
	void DepthFirstSearch();
	void BreadthFirstSearch();
	void OnCompletion();

public:
	const enum Algorithms
	{
		NONE = 0,
		DFS,
		BFS,
		DIJKSTRA,
		ASTAR
	};

	Maze* m_Maze = nullptr;
	bool m_Completed = false;
	Algorithms m_SelectedAlgorithm = Algorithms::NONE;
	// It can't access application class
	std::pair<uint32_t, uint32_t>* m_Route;

	//For DFS
	std::stack<uint32_t> m_Stack;
	//For BFS
	std::queue<uint32_t> m_Queue;
	std::vector<uint32_t> m_Path;
	std::vector<uint32_t> m_Parent;
};