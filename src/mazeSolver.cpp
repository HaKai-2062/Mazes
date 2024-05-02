#include <iostream>

#include "maze.h"
#include "mazeSolver.h"

MazeSolver::MazeSolver(Maze* maze, uint8_t selectedAlgorithm, std::pair<uint32_t, uint32_t>& route)
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

MazeSolver::~MazeSolver()
{
	while (!m_Stack.empty())
		m_Stack.pop();

	while (!m_Queue.empty())
		m_Queue.pop();
}

void MazeSolver::DepthFirstSearch()
{
	if (m_Stack.empty())
	{
		//std::cout << "Stack empty sus!\n";
		return;
	}

	// To get negative results for checks
	int64_t currentCell = static_cast<int64_t>(m_Stack.top());
	std::vector<uint8_t> neighbours;

	uint32_t northIndex = currentCell - 1;
	uint32_t eastIndex = currentCell + (m_Maze->m_CellsAcrossHeight);
	int64_t southIndex = currentCell + 1;
	int64_t westIndex = currentCell - (m_Maze->m_CellsAcrossHeight);

	// North
	if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 &&
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
	if ((southIndex % m_Maze->m_CellsAcrossHeight) != 0 &&
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

void MazeSolver::BreadthFirstSearch()
{
	std::vector<uint8_t> neighbours;

	if (m_Queue.empty())
	{
		//std::cout << "Queue empty sus!\n";
		return;
	}

	// To get negative results for checks
	int64_t currentCell = static_cast<int64_t>(m_Queue.front());
	uint32_t northIndex = currentCell - 1;
	uint32_t eastIndex = currentCell + (m_Maze->m_CellsAcrossHeight);
	int64_t southIndex = currentCell + 1;
	int64_t westIndex = currentCell - (m_Maze->m_CellsAcrossHeight);

	m_Queue.pop();

	// North
	if ((currentCell % m_Maze->m_CellsAcrossHeight) != 0 &&
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
	if ((southIndex % m_Maze->m_CellsAcrossHeight) != 0 &&
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

void MazeSolver::OnCompletion()
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