#pragma once

#include <stack>


namespace MazeBuilder
{
	void RecursiveBacktrack(std::map<uint32_t, uint16_t>& visitedCellInfo, std::stack<uint32_t>& stack, float totalCellWidth, uint32_t& visitedCellCount)
	{
		std::vector<uint8_t> neighbours;

		uint16_t cellsInOneAxis = static_cast<uint16_t>(2 / totalCellWidth);

		// North
		if ((stack.top() + 1) % cellsInOneAxis != 0 && (visitedCellInfo[stack.top() + 1] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(0);
		}
		// East, casted to avoid warning
		if (stack.top() < static_cast<uint32_t>(cellsInOneAxis * cellsInOneAxis - cellsInOneAxis) && (visitedCellInfo[stack.top() + cellsInOneAxis] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(1);
		}
		// South
		if (stack.top() % cellsInOneAxis != 0 && (visitedCellInfo[stack.top() - 1] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(2);
		}
		// West, casted to avoid warning
		if (stack.top() > static_cast<uint32_t>(cellsInOneAxis-1) && (visitedCellInfo[stack.top() - cellsInOneAxis] & Maze::CELL_VISITED) == 0)
		{
			neighbours.push_back(3);
		}

		if (!neighbours.empty())
		{
			int cellToVisit = neighbours[rand() % neighbours.size()];

			switch (cellToVisit)
			{

			case 0:
				visitedCellInfo[stack.top() + 1] |= Maze::CELL_VISITED | Maze::CELL_SOUTH;
				visitedCellInfo[stack.top()] |= Maze::CELL_NORTH;
				stack.push(stack.top() + 1);
				break;

			case 1:
				visitedCellInfo[stack.top() + cellsInOneAxis] |= Maze::CELL_VISITED | Maze::CELL_WEST;
				visitedCellInfo[stack.top()] |= Maze::CELL_EAST;
				stack.push(stack.top() + cellsInOneAxis);
				break;

			case 2:
				visitedCellInfo[stack.top() - 1] |= Maze::CELL_VISITED | Maze::CELL_NORTH;
				visitedCellInfo[stack.top()] |= Maze::CELL_SOUTH;
				stack.push(stack.top() - 1);
				break;

			case 3:
				visitedCellInfo[stack.top() - cellsInOneAxis] |= Maze::CELL_VISITED | Maze::CELL_EAST;
				visitedCellInfo[stack.top()] |= Maze::CELL_WEST;
				stack.push(stack.top() - cellsInOneAxis);
				break;
			}

			visitedCellCount++;
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