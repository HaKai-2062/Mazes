#pragma once

#include <stack>

namespace MazeBuilder
{
	void RecursiveBacktrack(Maze& maze, std::stack<uint32_t>& stack)
	{
		std::vector<uint8_t> neighbours;

		// To get negative results for checks
		int64_t currentCell = static_cast<int64_t>(stack.top());

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

		if (!neighbours.empty())
		{
			int cellToVisit = neighbours[rand() % neighbours.size()];

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
}

namespace MazeSolver
{

}