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

void getAllNeighbours(Maze& maze, std::vector<uint8_t>& neighbours, int64_t currentCell);

namespace MazeBuilder
{
	void RecursiveBacktrack(Maze& maze, std::stack<uint32_t>& stack)
	{
		std::vector<uint8_t> neighbours;

		// To get negative results for checks
		int64_t currentCell = static_cast<int64_t>(stack.top());

		getAllNeighbours(maze, neighbours, currentCell);

		if (!neighbours.empty())
		{
			uint8_t cellToVisit = neighbours[rand() % neighbours.size()];

			switch (cellToVisit)
			{
			case 0:
				maze.m_VisitedCellInfo[currentCell + 1] |= Maze::CELL_VISITED | Maze::CELL_SOUTH;
				maze.m_VisitedCellInfo[currentCell] |= Maze::CELL_NORTH;
				stack.push(currentCell + 1);
				break;

			case 1:
				maze.m_VisitedCellInfo[currentCell + maze.m_CellsAcrossHeight] |= Maze::CELL_VISITED | Maze::CELL_WEST;
				maze.m_VisitedCellInfo[currentCell] |= Maze::CELL_EAST;
				stack.push(currentCell + maze.m_CellsAcrossHeight);
				break;

			case 2:
				maze.m_VisitedCellInfo[currentCell - 1] |= Maze::CELL_VISITED | Maze::CELL_NORTH;
				maze.m_VisitedCellInfo[currentCell] |= Maze::CELL_SOUTH;
				stack.push(currentCell - 1);
				break;

			case 3:
				maze.m_VisitedCellInfo[currentCell - maze.m_CellsAcrossHeight] |= Maze::CELL_VISITED | Maze::CELL_EAST;
				maze.m_VisitedCellInfo[currentCell] |= Maze::CELL_WEST;
				stack.push(currentCell - maze.m_CellsAcrossHeight);
				break;
			}

			maze.m_VisitedCellCount++;
		}
		else
		{
			stack.pop();
		}
	}

	void RandomizedKruskal(Maze& maze, std::vector<std::pair<int32_t, int32_t>>& walls, DisjointSet& cells, std::vector<uint32_t>& wallShuffler)
	{
		if (wallShuffler.size() <= 0)
			maze.m_VisitedCellCount = maze.m_CellsAcrossWidth*maze.m_CellsAcrossHeight;
	
		uint32_t index = wallShuffler.back();		
		wallShuffler.pop_back();

		std::pair<int32_t, int32_t> wallToPop = walls[index];
		// dont erase from wall to keep the indexes relevant
		// walls.erase(walls.begin()+index);

		// When parent is distinct
		if (wallToPop.second != -1 && cells.Find(static_cast<uint32_t>(wallToPop.first)) != cells.Find(static_cast<uint32_t>(wallToPop.second)))
		{
			if (index < static_cast<uint64_t>(maze.m_CellsAcrossHeight * maze.m_CellsAcrossWidth))
			{
				maze.m_VisitedCellInfo[wallToPop.first] |= Maze::CELL_VISITED | Maze::CELL_SOUTH;
				maze.m_VisitedCellInfo[wallToPop.second] |= Maze::CELL_VISITED | Maze::CELL_NORTH;
			}
			else if (index >= static_cast<uint64_t>(maze.m_CellsAcrossHeight * maze.m_CellsAcrossWidth))
			{
				maze.m_VisitedCellInfo[wallToPop.first] |= Maze::CELL_VISITED | Maze::CELL_EAST;
				maze.m_VisitedCellInfo[wallToPop.second] |= Maze::CELL_VISITED | Maze::CELL_WEST;
			}

			cells.UnionSets(wallToPop.first, wallToPop.second);
		}
	}
}

namespace MazeSolver
{

}

void getAllNeighbours(Maze& maze, std::vector<uint8_t>& neighbours, int64_t currentCell)
{
	// North
	if (((currentCell + 1) % maze.m_CellsAcrossHeight) != 0 && (maze.m_VisitedCellInfo[currentCell + 1] & Maze::CELL_VISITED) == 0)
	{
		neighbours.push_back(0);
	}
	// East
	if ((currentCell + maze.m_CellsAcrossHeight) < (maze.m_CellsAcrossWidth * maze.m_CellsAcrossHeight) && (maze.m_VisitedCellInfo[currentCell + maze.m_CellsAcrossHeight] & Maze::CELL_VISITED) == 0)
	{
		neighbours.push_back(1);
	}
	// South
	if ((currentCell % maze.m_CellsAcrossHeight) != 0 && (maze.m_VisitedCellInfo[currentCell - 1] & Maze::CELL_VISITED) == 0)
	{
		neighbours.push_back(2);
	}
	// West
	if (currentCell - maze.m_CellsAcrossHeight >= 0 && (maze.m_VisitedCellInfo[currentCell - maze.m_CellsAcrossHeight] & Maze::CELL_VISITED) == 0)
	{
		neighbours.push_back(3);
	}
}