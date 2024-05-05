#include <queue>
#include <stack>
#include <unordered_set>

class Maze;

struct WeightDetails
{
	uint32_t id = 0;
	// Weights are set as infinity at start
	// For Dijkstra weight = cost
	// For A*, weight = cost + heuristic function (Euclidian distance)
	uint32_t weight = std::numeric_limits<uint32_t>::max();
};

struct CompareWeights
{
	bool operator()(const WeightDetails l, const WeightDetails r) const { return l.weight > r.weight; }
};

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
	void DijkstraSearch();
	void AstarSearch();
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
	std::vector<uint32_t> m_Path;
	std::vector<uint32_t> m_Parent;

	//For DFS
	std::stack<uint32_t> m_Stack;
	//For BFS
	std::queue<uint32_t> m_Queue;

	// For Dijkstra, A-star
	std::priority_queue<WeightDetails, std::vector<WeightDetails>, CompareWeights> m_PQueue;
	std::vector<uint32_t> m_Distance;

	// For A-star
};