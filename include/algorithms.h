#pragma once

#include <stack>
#include <list>
#include <set>

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

		// North
		if (((currentCell + 1) % m_Maze->m_CellsAcrossHeight) != 0 && (m_Maze->m_VisitedCellInfo[currentCell + 1] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(0);
		}
		// East
		if ((currentCell + m_Maze->m_CellsAcrossHeight) < (m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight) && (m_Maze->m_VisitedCellInfo[currentCell + m_Maze->m_CellsAcrossHeight] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(1);
		}
		// South
		if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 && (m_Maze->m_VisitedCellInfo[currentCell - 1] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(2);
		}
		// West
		if (currentCell - m_Maze->m_CellsAcrossHeight >= 0 && (m_Maze->m_VisitedCellInfo[currentCell - m_Maze->m_CellsAcrossHeight] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(3);
		}

		if (!neighbours.empty())
		{
			uint8_t cellToVisit = neighbours[rand() % neighbours.size()];

			switch (cellToVisit)
			{
			case 0:
				m_Maze->m_VisitedCellInfo[currentCell + 1] |= Maze::CELL_VISITED | Maze::CELL_SOUTH;
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_NORTH;
				m_Stack.push(currentCell + 1);
				break;

			case 1:
				m_Maze->m_VisitedCellInfo[currentCell + m_Maze->m_CellsAcrossHeight] |= Maze::CELL_VISITED | Maze::CELL_WEST;
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_EAST;
				m_Stack.push(currentCell + m_Maze->m_CellsAcrossHeight);
				break;

			case 2:
				m_Maze->m_VisitedCellInfo[currentCell - 1] |= Maze::CELL_VISITED | Maze::CELL_NORTH;
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_SOUTH;
				m_Stack.push(currentCell - 1);
				break;

			case 3:
				m_Maze->m_VisitedCellInfo[currentCell - m_Maze->m_CellsAcrossHeight] |= Maze::CELL_VISITED | Maze::CELL_EAST;
				m_Maze->m_VisitedCellInfo[currentCell] |= Maze::CELL_WEST;
				m_Stack.push(currentCell - m_Maze->m_CellsAcrossHeight);
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
		if (m_WallShuffler.size() <= 0)
			m_Maze->m_VisitedCellCount = m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight;

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
				m_Maze->m_VisitedCellInfo[wallToPop.first] |= Maze::CELL_VISITED | Maze::CELL_SOUTH;
				m_Maze->m_VisitedCellInfo[wallToPop.second] |= Maze::CELL_VISITED | Maze::CELL_NORTH;
			}
			else if (index >= static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
			{
				m_Maze->m_VisitedCellInfo[wallToPop.first] |= Maze::CELL_VISITED | Maze::CELL_EAST;
				m_Maze->m_VisitedCellInfo[wallToPop.second] |= Maze::CELL_VISITED | Maze::CELL_WEST;
			}

			m_Cells->UnionSets(wallToPop.first, wallToPop.second);
		}
	}

public:
	enum Algorithms
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
	DisjointSet* m_Cells;
	// Index is wallNumber, pair are cells separated by that wall
	// Each cell has 2 walls by default in N->S and W->E
	std::vector<std::pair<int32_t, int32_t>> walls;
	// These represent the indexes
	std::vector<uint32_t> m_WallShuffler;
};

/*
class MazeSolver
{

}
*/
