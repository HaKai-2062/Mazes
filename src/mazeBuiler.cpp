#include <iostream>
#include <vector>
#include <random>
#include <numeric>

#include "maze.h"
#include "mazeBuilder.h"

MazeBuilder::MazeBuilder(Maze* maze, uint8_t selectedAlgorithm)
	: m_Maze(maze)
{
	m_StartCoordinate = rand() % m_Maze->m_MazeArea;
	m_SelectedAlgorithm = static_cast<Algorithms>(selectedAlgorithm);
	m_Walls.resize(2 * m_Maze->m_MazeArea);

	switch (m_SelectedAlgorithm)
	{
	case Algorithms::RECURSIVE_BACKTRACK:
	{
		m_Stack.push(m_StartCoordinate);
		break;
	}
	case Algorithms::KRUSKAL:
	{
		m_Cells = new DisjointSet(maze->m_MazeArea);

		// Add south walls first
		for (uint32_t i = 0; i < m_Maze->m_MazeArea; i++)
		{
			if (i % m_Maze->m_CellsAcrossHeight == 0)
				m_Walls[i] = std::make_pair<int32_t, int32_t>(i, -1);
			else
				m_Walls[i] = std::make_pair<int32_t, int32_t>(i, i - 1);
		}

		// Add east walls now
		for (uint32_t i = m_Maze->m_MazeArea; i < m_Walls.size(); i++)
		{
			// This makes the walls index to start from 0
			uint32_t normalizedEastWall = i - m_Maze->m_MazeArea;

			if (normalizedEastWall >= m_Maze->m_MazeArea - m_Maze->m_CellsAcrossHeight)
				m_Walls[i] = std::make_pair<int32_t, int32_t>(normalizedEastWall, -1);
			else
				m_Walls[i] = std::make_pair<int32_t, int32_t>(normalizedEastWall, normalizedEastWall + m_Maze->m_CellsAcrossHeight);
		}

		std::random_device rd;
		std::mt19937 g(rd());
		m_WallShuffler.resize(2 * m_Maze->m_MazeArea);
		m_WallShuffler[0] = 0;
		std::iota(m_WallShuffler.begin() + 1, m_WallShuffler.end() - 1, 1);
		std::shuffle(m_WallShuffler.begin(), m_WallShuffler.end(), g);
		break;
	}
	case Algorithms::PRIMS:
	{
		// Add south walls first
		for (uint32_t i = 0; i < m_Maze->m_MazeArea; i++)
		{
			if (i % m_Maze->m_CellsAcrossHeight == 0)
				m_Walls[i] = std::make_pair<int32_t, int32_t>(i, -1);
			else
				m_Walls[i] = std::make_pair<int32_t, int32_t>(i, i - 1);
		}

		// Add east walls now
		for (uint32_t i = m_Maze->m_MazeArea; i < m_Walls.size(); i++)
		{
			// This makes the walls index to start from 0
			uint32_t normalizedEastWall = i - m_Maze->m_MazeArea;

			if (normalizedEastWall >= m_Maze->m_MazeArea - m_Maze->m_CellsAcrossHeight)
				m_Walls[i] = std::make_pair<int32_t, int32_t>(normalizedEastWall, -1);
			else
				m_Walls[i] = std::make_pair<int32_t, int32_t>(normalizedEastWall, normalizedEastWall + m_Maze->m_CellsAcrossHeight);
		}

		m_WallShuffler.clear();
		m_WallShuffler.reserve(2 * m_Maze->m_MazeArea);

		// S
		if (((m_StartCoordinate + 1) % m_Maze->m_CellsAcrossHeight) != 0)
			m_WallShuffler.push_back(m_StartCoordinate + 1);
		// N
		m_WallShuffler.push_back(m_StartCoordinate);
		// E
		m_WallShuffler.push_back(m_Maze->m_MazeArea + m_StartCoordinate);
		// W
		if (m_StartCoordinate >= m_Maze->m_CellsAcrossHeight)
			m_WallShuffler.push_back((static_cast<int32_t>(m_Maze->m_MazeArea) - static_cast<int32_t>(m_Maze->m_CellsAcrossHeight)) + m_StartCoordinate);

		m_Maze->m_VisitedCellInfo[m_StartCoordinate] |= Maze::CELL_VISITED;

		m_LastCell = m_StartCoordinate;
		break;
	}
	}
}

void MazeBuilder::OnCompletion()
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
	case Algorithms::PRIMS:
		break;
	}
}

void MazeBuilder::RecursiveBacktrack()
{
	std::vector<uint8_t> neighbours;

	// To get negative results for checks
	int64_t currentCell = static_cast<int64_t>(m_Stack.top());

	uint32_t northIndex = currentCell - 1;
	uint32_t eastIndex = currentCell + (m_Maze->m_CellsAcrossHeight);
	uint32_t southIndex = currentCell + 1;
	int64_t westIndex = currentCell - (m_Maze->m_CellsAcrossHeight);

	// North
	if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 && (m_Maze->m_VisitedCellInfo[northIndex] & Maze::CELL_VISITED) == 0)
	{
		neighbours.push_back(0);
	}
	// East
	if (eastIndex < (static_cast<uint64_t>(m_Maze->m_CellsAcrossWidth) * m_Maze->m_CellsAcrossHeight) && (m_Maze->m_VisitedCellInfo[eastIndex] & Maze::CELL_VISITED) == 0)
	{
		neighbours.push_back(1);
	}
	// South
	if ((southIndex % m_Maze->m_CellsAcrossHeight) != 0 && (m_Maze->m_VisitedCellInfo[southIndex] & Maze::CELL_VISITED) == 0)
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

void MazeBuilder::RandomizedKruskal()
{
	if (m_WallShuffler.empty())
	{
		m_Maze->m_VisitedCellCount = m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight;
		return;
	}

	uint32_t index = m_WallShuffler.back();
	m_WallShuffler.pop_back();

	std::pair<int32_t, int32_t> wallToPop = m_Walls[index];
	// dont erase from wall to keep the indexes relevant
	// walls.erase(walls.begin()+index);

	// When parent is distinct
	if (wallToPop.second != -1 && m_Cells->Find(static_cast<uint32_t>(wallToPop.first)) != m_Cells->Find(static_cast<uint32_t>(wallToPop.second)))
	{
		if (index < static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
		{
			m_Maze->m_VisitedCellInfo[wallToPop.first] |= (Maze::CELL_VISITED | Maze::CELL_NORTH);
			m_Maze->m_VisitedCellInfo[wallToPop.second] |= (Maze::CELL_VISITED | Maze::CELL_SOUTH);
		}
		else if (index >= static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
		{
			m_Maze->m_VisitedCellInfo[wallToPop.first] |= (Maze::CELL_VISITED | Maze::CELL_EAST);
			m_Maze->m_VisitedCellInfo[wallToPop.second] |= (Maze::CELL_VISITED | Maze::CELL_WEST);
		}

		m_Cells->UnionSets(wallToPop.first, wallToPop.second);
	}
}

void MazeBuilder::RandomizedPrims()
{
	if (m_WallShuffler.empty())
	{
		m_Maze->m_VisitedCellCount = m_Maze->m_CellsAcrossWidth * m_Maze->m_CellsAcrossHeight;
		return;
	}
	/*
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(m_WallShuffler.begin(), m_WallShuffler.end(), g);

	uint32_t index = m_WallShuffler.back();
	m_WallShuffler.pop_back();
	*/

	uint32_t wallShufflerIndex = rand() % m_WallShuffler.size();
	uint32_t index = m_WallShuffler[wallShufflerIndex];
	m_WallShuffler.erase(m_WallShuffler.begin() + wallShufflerIndex);

	std::pair<int32_t, int32_t> wallToPop = m_Walls[index];

	if (wallToPop.second == -1)
	{
		//std::cout << "Out of bound\n";
		return;
	}

	if ((m_Maze->m_VisitedCellInfo[wallToPop.second] & Maze::CELL_VISITED) != 0 && (m_Maze->m_VisitedCellInfo[wallToPop.first] & Maze::CELL_VISITED) != 0)
	{
		//std::cout << "Visited already\n";
		return;
	}

	// Always stores the cell which wasnt visited by this route
	if ((m_Maze->m_VisitedCellInfo[wallToPop.second] & Maze::CELL_VISITED) != 0)
		m_LastCell = wallToPop.first;
	else
		m_LastCell = wallToPop.second;

	if (index < static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
	{
		m_Maze->m_VisitedCellInfo[wallToPop.first] |= (Maze::CELL_VISITED | Maze::CELL_NORTH);
		m_Maze->m_VisitedCellInfo[wallToPop.second] |= (Maze::CELL_VISITED | Maze::CELL_SOUTH);

		if (m_LastCell == wallToPop.second)
		{
			// N 
			m_WallShuffler.push_back(m_LastCell);
		}
		else
		{
			// S
			if (((m_LastCell + 1) % m_Maze->m_CellsAcrossHeight) != 0)
				m_WallShuffler.push_back(m_LastCell + 1);
		}

		// E
		m_WallShuffler.push_back(m_Maze->m_MazeArea + m_LastCell);
		// W
		if (m_LastCell >= m_Maze->m_CellsAcrossHeight)
			m_WallShuffler.push_back((static_cast<int64_t>(m_Maze->m_MazeArea) - m_Maze->m_CellsAcrossHeight) + m_LastCell);
	}
	else if (index >= static_cast<uint64_t>(m_Maze->m_CellsAcrossHeight * m_Maze->m_CellsAcrossWidth))
	{
		m_Maze->m_VisitedCellInfo[wallToPop.first] |= (Maze::CELL_VISITED | Maze::CELL_EAST);
		m_Maze->m_VisitedCellInfo[wallToPop.second] |= (Maze::CELL_VISITED | Maze::CELL_WEST);

		// S
		if (((m_LastCell + 1) % m_Maze->m_CellsAcrossHeight) != 0)
			m_WallShuffler.push_back(m_LastCell + 1);
		// N
		m_WallShuffler.push_back(m_LastCell);
		if (m_LastCell == wallToPop.second)
		{
			// E
			m_WallShuffler.push_back(m_Maze->m_MazeArea + m_LastCell);
		}
		else
		{
			// W
			if (m_LastCell >= m_Maze->m_CellsAcrossHeight)
				m_WallShuffler.push_back((static_cast<int64_t>(m_Maze->m_MazeArea) - m_Maze->m_CellsAcrossHeight) + m_LastCell);
		}
	}
}

DisjointSet::DisjointSet(uint32_t n)
{
	parent.resize(n);
	rank.resize(n, 0);

	if (n == 0)
		return;

	parent[0] = 0;
	std::iota(parent.begin() + 1, parent.end(), 1);
}

uint32_t DisjointSet::Find(uint32_t x)
{
	if (parent[x] != x)
	{
		// Path compression: Make every visited node point directly to the root.
		parent[x] = Find(parent[x]);
	}
	return parent[x];
}

void DisjointSet::UnionSets(uint32_t x, uint32_t y)
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