#pragma once

#include <stack>
#include <list>
#include <set>
#include <queue>

class DisjointSet
{
private:
	std::vector<uint32_t> parent;
	std::vector<uint32_t> rank;
public:
	DisjointSet() = delete;
	DisjointSet(DisjointSet&) = delete;
	DisjointSet(DisjointSet&&) = delete;

	DisjointSet(uint32_t n)
	{
		parent.resize(n);
		rank.resize(n, 0);
		
		if (n == 0)
			return;
		
		parent[0] = 0;
		std::iota(parent.begin()+1, parent.end(), 1);
	}

	uint32_t Find(uint32_t x)
	{
		if (parent[x] != x)
		{
			// Path compression: Make every visited node point directly to the root.
			parent[x] = Find(parent[x]);
		}
		return parent[x];
	}

	void UnionSets(uint32_t x, uint32_t y)
	{
		uint32_t rootX = Find(x);
		uint32_t rootY = Find(y);

		if (rootX == rootY) return;

		if (rank[rootX] < rank[rootY])
		{
			parent[rootX] = rootY;
		}
		else if (rank[rootX] > rank[rootY])
		{
			parent[rootY] = rootX;
		}
		else
		{
			parent[rootY] = rootX;
			rank[rootX]++;
		}
	}
};

class MazeBuilder
{
public:

	MazeBuilder() = delete;
	MazeBuilder(MazeBuilder&) = delete;
	MazeBuilder(MazeBuilder&&) = delete;

	MazeBuilder(Maze* maze, uint8_t selectedAlgorithm)
		: m_Maze(maze)
	{
		uint32_t startCoordinate = rand() % m_Maze->m_MazeArea;
		m_SelectedAlgorithm = static_cast<Algorithms>(selectedAlgorithm);

		switch (m_SelectedAlgorithm)
		{
		case Algorithms::RECURSIVE_BACKTRACK:
			m_Stack.push(startCoordinate);
			break;
		case Algorithms::KRUSKAL:
			m_Cells = new DisjointSet(maze->m_MazeArea);
			walls.resize(2 * m_Maze->m_MazeArea);

			// Add south walls first
			for (uint32_t i = 0; i < m_Maze->m_MazeArea; i++)
			{
				if (i % m_Maze->m_CellsAcrossHeight == 0)
					walls[i] = std::make_pair<int32_t, int32_t>(i, -1);
				else
					walls[i] = std::make_pair<int32_t, int32_t>(i, i - 1);
			}

			// Add east walls now
			for (uint32_t i = m_Maze->m_MazeArea; i < walls.size(); i++)
			{
				// This makes the walls index to start from 0
				uint32_t normalizedEastWall = i - m_Maze->m_MazeArea;

				if (normalizedEastWall >= m_Maze->m_MazeArea - m_Maze->m_CellsAcrossHeight)
					walls[i] = std::make_pair<int32_t, int32_t>(normalizedEastWall, -1);
				else
					walls[i] = std::make_pair<int32_t, int32_t>(normalizedEastWall, normalizedEastWall + m_Maze->m_CellsAcrossHeight);
			}

			std::random_device rd;
			std::mt19937 g(rd());
			m_WallShuffler.resize(2 * m_Maze->m_MazeArea);
			m_WallShuffler[0] = 0;
			std::iota(m_WallShuffler.begin() + 1, m_WallShuffler.end() - 1, 1);
			std::shuffle(m_WallShuffler.begin(), m_WallShuffler.end(), g);
			break;
		}
	}

	void OnCompletion()
	{
		switch (m_SelectedAlgorithm)
		{
		case Algorithms::RECURSIVE_BACKTRACK:
			// Stack probably holds the waypoint from start to end
			// so we clear the stack
			while (!m_Stack.empty())
				m_Stack.pop();

			break;
		case Algorithms::KRUSKAL:
			break;
		}
	}

	void RecursiveBacktrack()
	{
		std::vector<uint8_t> neighbours;

		// To get negative results for checks
		int64_t currentCell = static_cast<int64_t>(m_Stack.top());

		uint32_t northIndex = currentCell + 1;
		uint32_t eastIndex = currentCell + (m_Maze->m_CellsAcrossHeight);
		uint32_t southIndex = currentCell - 1;
		int64_t westIndex = currentCell - (m_Maze->m_CellsAcrossHeight);

		// North
		if ((northIndex % m_Maze->m_CellsAcrossHeight) != 0 && (m_Maze->m_VisitedCellInfo[northIndex] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(0);
		}
		// East
		if (eastIndex < (static_cast<uint64_t>(m_Maze->m_CellsAcrossWidth) * m_Maze->m_CellsAcrossHeight) && (m_Maze->m_VisitedCellInfo[eastIndex] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(1);
		}
		// South
		if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 && (m_Maze->m_VisitedCellInfo[southIndex] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(2);
		}
		// West
		if (westIndex >= 0 && (m_Maze->m_VisitedCellInfo[westIndex] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(3);
		}

		if (!neighbours.empty())
		{
			uint8_t cellToVisit = neighbours[rand() % neighbours.size()];

			switch (cellToVisit)
			{
			case 0:
				m_Maze->m_VisitedCellInfo[northIndex] |= (Maze::CELL_VISITED | Maze::CELL_SOUTH);
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_NORTH;
				m_Stack.push(northIndex);
				break;

			case 1:
				m_Maze->m_VisitedCellInfo[eastIndex] |= (Maze::CELL_VISITED | Maze::CELL_WEST);
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_EAST;
				m_Stack.push(eastIndex);
				break;

			case 2:
				m_Maze->m_VisitedCellInfo[southIndex] |= (Maze::CELL_VISITED | Maze::CELL_NORTH);
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_SOUTH;
				m_Stack.push(southIndex);
				break;

			case 3:
				m_Maze->m_VisitedCellInfo[westIndex] |= (Maze::CELL_VISITED | Maze::CELL_EAST);
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_WEST;
				m_Stack.push(westIndex);
				break;
			}

			m_Maze->m_VisitedCellCount++;
		}
		else
		{
			m_Stack.pop();
		}
	}

	void RandomizedKruskal()
	{
		if (m_WallShuffler.empty())
		{
			m_Maze->m_VisitedCellCount = m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight;
			return;
		}

		uint32_t index = m_WallShuffler.back();
		m_WallShuffler.pop_back();

		std::pair<int32_t, int32_t> wallToPop = walls[index];
		// dont erase from wall to keep the indexes relevant
		// walls.erase(walls.begin()+index);

		// When parent is distinct
		if (wallToPop.second != -1 && m_Cells->Find(static_cast<uint32_t>(wallToPop.first)) != m_Cells->Find(static_cast<uint32_t>(wallToPop.second)))
		{
			if (index < static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
			{
				m_Maze->m_VisitedCellInfo[wallToPop.first] |= (Maze::CELL_VISITED | Maze::CELL_SOUTH);
				m_Maze->m_VisitedCellInfo[wallToPop.second] |= (Maze::CELL_VISITED | Maze::CELL_NORTH);
			}
			else if (index >= static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
			{
				m_Maze->m_VisitedCellInfo[wallToPop.first] |= (Maze::CELL_VISITED | Maze::CELL_EAST);
				m_Maze->m_VisitedCellInfo[wallToPop.second] |= (Maze::CELL_VISITED | Maze::CELL_WEST);
			}

			m_Cells->UnionSets(wallToPop.first, wallToPop.second);
		}
	}

public:
	const enum Algorithms
	{
		RECURSIVE_BACKTRACK = 0,
		KRUSKAL
	};

	Maze* m_Maze = nullptr;
	bool m_Completed = false;
	Algorithms m_SelectedAlgorithm = Algorithms::RECURSIVE_BACKTRACK;
	uint32_t m_StartCoordinate = 0;

	// For RECURSIVE_BACKTRACK
	std::stack<uint32_t> m_Stack;

	// For KRUSKAL
	// Cells indxed by the cell number
	DisjointSet* m_Cells = nullptr;
	// Index is wallNumber, pair are cells separated by that wall
	// Each cell has 2 walls by default in N->S and W->E
	std::vector<std::pair<int32_t, int32_t>> walls;
	// These represent the indexes
	std::vector<uint32_t> m_WallShuffler;
};

class MazeSolver
{
public:
	MazeSolver() = delete;
	MazeSolver(MazeSolver&) = delete;
	MazeSolver(MazeSolver&&) = delete;

	MazeSolver(Maze* maze, uint8_t selectedAlgorithm, std::pair<uint32_t, uint32_t>& route)
		:m_Maze(maze)
	{
		m_SelectedAlgorithm = static_cast<Algorithms>(selectedAlgorithm);

		switch (m_SelectedAlgorithm)
		{
		case Algorithms::DFS:
			m_Stack.push(route.first);
			break;
		case Algorithms::BFS:
			m_Queue.push(route.first);
			break;
		}

		m_Parent.resize(m_Maze->m_MazeArea);
		m_Route = &route;
	}

	~MazeSolver()
	{
		while(!m_Stack.empty())
			m_Stack.pop();
	
		while(!m_Queue.empty())
			m_Queue.pop();
	}

	void DepthFirstSearch()
	{
		if (m_Stack.empty())
		{
			//std::cout << "Stack empty sus!\n";
			return;
		}

		// To get negative results for checks
 		int64_t currentCell = static_cast<int64_t>(m_Stack.top());
		std::vector<uint8_t> neighbours;

		uint32_t northIndex = currentCell + 1;
		uint32_t eastIndex = currentCell + (m_Maze->m_CellsAcrossHeight);
		int64_t southIndex = currentCell - 1;
		int64_t westIndex = currentCell - (m_Maze->m_CellsAcrossHeight);

		// North
		if ((northIndex % m_Maze->m_CellsAcrossHeight) != 0 &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_NORTH) != 0 && (m_Maze->m_VisitedCellInfo[northIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(0);
		}
		// East
		if (eastIndex < (m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight) &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_EAST) != 0 && (m_Maze->m_VisitedCellInfo[eastIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(1);
		}
		// South
		if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 && 
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_SOUTH) != 0 && (m_Maze->m_VisitedCellInfo[southIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(2);
		}
		// West
		if (westIndex >= 0 &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_WEST) != 0 && (m_Maze->m_VisitedCellInfo[westIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(3);
		}

		if (!neighbours.empty())
		{
			uint8_t cellToVisit = neighbours[rand() % neighbours.size()];

			switch (cellToVisit)
			{
			case 0:
				m_Maze->m_VisitedCellInfo[northIndex] |= Maze::CELL_SEARCHED;
				m_Stack.push(northIndex);
				break;

			case 1:
				m_Maze->m_VisitedCellInfo[eastIndex] |= Maze::CELL_SEARCHED;
				m_Stack.push(eastIndex);
				break;

			case 2:
				m_Maze->m_VisitedCellInfo[southIndex] |= Maze::CELL_SEARCHED;
				m_Stack.push(southIndex);
				break;

			case 3:
				m_Maze->m_VisitedCellInfo[westIndex] |= Maze::CELL_SEARCHED;
				m_Stack.push(westIndex);
				break;
			}
		}
		else
		{
			m_Stack.pop();
		}
	}

	void BreadthFirstSearch()
	{
		std::vector<uint8_t> neighbours;

		if (m_Queue.empty())
		{
			//std::cout << "Queue empty sus!\n";
			return;
		}

		// To get negative results for checks
		int64_t currentCell = static_cast<int64_t>(m_Queue.front());
		uint32_t northIndex = currentCell + 1;
		uint32_t eastIndex = currentCell + (m_Maze->m_CellsAcrossHeight);
		int64_t southIndex = currentCell - 1;
		int64_t westIndex = currentCell - (m_Maze->m_CellsAcrossHeight);
		
		m_Queue.pop();

		// North
		if ((northIndex % m_Maze->m_CellsAcrossHeight) != 0 &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_NORTH) != 0 && (m_Maze->m_VisitedCellInfo[northIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(0);
		}
		// East
		if (eastIndex < (m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight) &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_EAST) != 0 && (m_Maze->m_VisitedCellInfo[eastIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(1);
		}
		// South
		if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_SOUTH) != 0 && (m_Maze->m_VisitedCellInfo[southIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(2);
		}
		// West
		if (westIndex >= 0 &&
			(m_Maze->m_VisitedCellInfo[currentCell] & Maze::CELL_WEST) != 0 && (m_Maze->m_VisitedCellInfo[westIndex] & Maze::CELL_SEARCHED) == 0)
		{
			neighbours.push_back(3);
		}

		if (!neighbours.empty())
		{
			//std::shuffle(std::begin(neighbours), std::end(neighbours), std::mt19937{ std::random_device{}() });

			for (uint8_t i = 0; i < neighbours.size(); i++)
			{
				uint8_t cellToVisit = neighbours[i];
				switch (cellToVisit)
				{
					case 0:
					{
						m_Maze->m_VisitedCellInfo[northIndex] |= Maze::CELL_SEARCHED;
						m_Queue.push(northIndex);
						m_Parent[northIndex] = currentCell;
						break;
					}

					case 1:
					{
						m_Maze->m_VisitedCellInfo[eastIndex] |= Maze::CELL_SEARCHED;
						m_Queue.push(eastIndex);
						m_Parent[eastIndex] = currentCell;
						break;
					}

					case 2:
					{
						m_Maze->m_VisitedCellInfo[southIndex] |= Maze::CELL_SEARCHED;
						m_Queue.push(southIndex);
						m_Parent[southIndex] = currentCell;
						break;
					}

					case 3:
					{
						m_Maze->m_VisitedCellInfo[westIndex] |= Maze::CELL_SEARCHED;
						m_Queue.push(westIndex);
						m_Parent[westIndex] = currentCell;
						break;
					}
				}
			}
		}
	}

	void OnCompletion()
	{
		if (m_SelectedAlgorithm == MazeSolver::Algorithms::DFS)
		{
			std::stack<uint32_t> tempStack = m_Stack;
			int32_t size = tempStack.size();
			m_Path.clear();
			m_Path.resize(tempStack.size());
			for (int32_t i = size - 1; i >= 0; i--)
			{
				m_Path[i] = tempStack.top();
				tempStack.pop();
			}
		}

		if (m_SelectedAlgorithm == MazeSolver::Algorithms::BFS)
		{
			uint32_t currentCell = m_Route->second;

			// Backtracking
			while (currentCell != m_Route->first)
			{
				m_Path.push_back(currentCell);
				currentCell = m_Parent[currentCell];
			}

			m_Path.push_back(m_Route->first);
			std::reverse(m_Path.begin(), m_Path.end());
		}
	}

public:
	const enum Algorithms
	{
		DFS = 0, BFS
	};

	Maze* m_Maze = nullptr;
	bool m_Completed = false;
	Algorithms m_SelectedAlgorithm = Algorithms::DFS;
	// It can't access application class
	std::pair<uint32_t, uint32_t>* m_Route;

	//For DFS
	std::stack<uint32_t> m_Stack;
	//For BFS
	std::queue<uint32_t> m_Queue;
	std::vector<uint32_t> m_Path;
	std::vector<uint32_t> m_Parent;
};
