#include <stack>

class Maze;
class DisjointSet;

class MazeBuilder
{
public:
	MazeBuilder() = delete;
	MazeBuilder(MazeBuilder&) = delete;
	MazeBuilder(MazeBuilder&&) = delete;

	MazeBuilder(Maze* maze, uint8_t selectedAlgorithm);
	//~MazeBuilder();
	
	void OnCompletion();
	void RecursiveBacktrack();
	void RandomizedKruskal();
	void RandomizedPrims();
	void Wilson();

public:
	const enum Algorithms
	{
		NONE = 0,
		RECURSIVE_BACKTRACK,
		KRUSKAL,
		PRIMS,
		WILSON
	};

	Maze* m_Maze = nullptr;
	bool m_Completed = false;
	Algorithms m_SelectedAlgorithm = Algorithms::NONE;
	uint32_t m_StartCoordinate = 0;

	// For RECURSIVE_BACKTRACK
	std::stack<uint32_t> m_Stack;

	// For KRUSKAL
	// Cells indxed by the cell number
	DisjointSet* m_Cells = nullptr;
	// Index is wallNumber, pair are cells separated by that wall
	// Each cell has 2 walls by default in N->S and W->E
	std::vector<std::pair<int32_t, int32_t>> m_Walls;
	// These represent the indexes
	std::vector<uint32_t> m_WallShuffler;
	uint32_t m_LastCell = 0;
};

class DisjointSet
{
public:
	DisjointSet() = delete;
	DisjointSet(DisjointSet&) = delete;
	DisjointSet(DisjointSet&&) = delete;

	DisjointSet(uint32_t n);

	uint32_t Find(uint32_t x);
	void UnionSets(uint32_t x, uint32_t y);
private:
	std::vector<uint32_t> parent;
	std::vector<uint32_t> rank;
};