#pragma once

#include <map>
#include <stack>

class Maze
{
public:

    Maze() = delete;
    Maze(Maze&) = delete;
    Maze(Maze&&) = delete;

    Maze(uint16_t width, uint16_t height)
        : m_MazeWidth(width), m_MazeHeight(height) 
    {
        std::cout << "Maze Created [" << width << "x" << height << "]" << std::endl;
        m_CellsAcrossWidth = static_cast<uint16_t>(floor(width / totalCellHeight));
        m_CellsAcrossHeight = static_cast<uint16_t>(floor(height / totalCellHeight));
        m_MazeArea = m_CellsAcrossHeight * m_CellsAcrossWidth;
    }

	~Maze()
	{
		std::cout << "Maze Destroyed" << std::endl;
	}

	uint32_t DrawMaze(std::vector<std::pair<float, float>>& vertices, std::vector<uint32_t>& indices, std::stack<uint32_t>* stack = nullptr)
	{
        uint32_t currentCell = 0;

        auto colorOfVertex = [&]()
        {
            // RG, BA color of each block
            if (stack && !stack->empty() && currentCell == stack->top())
            {
                vertices.push_back(std::make_pair<float, float>(0.0f, 1.0f));
                vertices.push_back(std::make_pair<float, float>(0.0f, 1.0f));
            }
            else if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && m_VisitedCellInfo[currentCell] & CELL_VISITED)
            {
                vertices.push_back(std::make_pair<float, float>(1.0f, 1.0f));
                vertices.push_back(std::make_pair<float, float>(1.0f, 1.0f));
            }
            else
            {
                vertices.push_back(std::make_pair<float, float>(1.0f, 0.5f));
                vertices.push_back(std::make_pair<float, float>(0.2f, 1.0f));
            }
        };

        // We are multiplying by 2 because the coordinate go from -1 to 1 instead of 0 to 1
        float normalizedHalfCellWidth = static_cast<float>(2 * m_HalfCellHeight) / (m_MazeWidth);
        float normalizedHalfCellHeight = static_cast<float>(2 * m_HalfCellHeight) / (m_MazeHeight);
        float normalizedWallThickness = static_cast<float>(2 * m_WallThickness) / (m_MazeHeight);

        float normalizedTotalCellWidth = static_cast<float>(2 * totalCellHeight) / (m_MazeWidth);
        float normalizedTotalCellHeight = static_cast<float>(2 * totalCellHeight) / (m_MazeHeight);

        // This goes from bottomRight (-1, -1) to topLeft (1, 1)
        // WIDTH
        for (float i = -1.0f; i < 1.0f; i += normalizedTotalCellWidth)
        {
            // To not draw out of bound cells
            if (i + 2 * normalizedHalfCellWidth > 1)
            {
                continue;
            }
            // HEIGHT
            for (float j = -1.0f; j < 1.0f; j += normalizedTotalCellHeight)
            {
                if (j + 2 * normalizedHalfCellHeight > 1)
                {
                    continue;
                }

                bool isWallEast = true;
                if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && m_VisitedCellInfo[currentCell] & CELL_EAST)
                    isWallEast = false;

                bool isWallSouth = true;
                if (m_VisitedCellInfo.find(currentCell) != m_VisitedCellInfo.end() && m_VisitedCellInfo[currentCell] & CELL_SOUTH)
                    isWallSouth = false;

                indices.push_back((4 * currentCell) + 0);
                indices.push_back((4 * currentCell) + 1);
                indices.push_back((4 * currentCell) + 3);
                indices.push_back((4 * currentCell) + 1);
                indices.push_back((4 * currentCell) + 2);
                indices.push_back((4 * currentCell) + 3);

                std::vector<std::pair<float, float>> m_CellOrigin;
                m_CellOrigin.push_back(std::make_pair<float, float>(i + normalizedHalfCellWidth, j + normalizedHalfCellHeight));

                // right top
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first + normalizedHalfCellWidth + (isWallEast ? 0 : normalizedWallThickness), m_CellOrigin.back().second + normalizedHalfCellHeight));
                colorOfVertex();
                // right bottom
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first + normalizedHalfCellWidth + (isWallEast ? 0 : normalizedWallThickness), m_CellOrigin.back().second - normalizedHalfCellHeight - (isWallSouth ? 0 : normalizedWallThickness)));
                colorOfVertex();
                // left bottom
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first - normalizedHalfCellWidth, m_CellOrigin.back().second - normalizedHalfCellHeight - (isWallSouth ? 0 : normalizedWallThickness)));
                colorOfVertex();
                // left top
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first - normalizedHalfCellWidth, m_CellOrigin.back().second + normalizedHalfCellHeight));
                colorOfVertex();

                if (m_VisitedCellInfo.find(currentCell) == m_VisitedCellInfo.end())
                {
                    m_VisitedCellInfo[currentCell] = 0x00;
                }

                currentCell++;
            }
        }

        return currentCell;
	}

    bool MazeCompleted()
    {
        return m_VisitedCellCount == m_CellsAcrossWidth * m_CellsAcrossHeight;
    }

public:
    // This is made public to let our algorithm access it
    // TDL: Setup appropriate getters/setters
    
    // 1st int is cell number,
    // 2nd int represents the neighbours and if it has been visited or not
    std::map<uint32_t, uint16_t> m_VisitedCellInfo;

    const enum
    {
        CELL_NORTH = 0x01,
        CELL_EAST = 0x02,
        CELL_SOUTH = 0x04,
        CELL_WEST = 0x08,
        CELL_VISITED = 0x10
    };

    // These values are in pixels
    uint16_t m_HalfCellHeight = 10;
    uint16_t m_WallThickness = 2;
    uint16_t totalCellHeight = 2 * m_HalfCellHeight + m_WallThickness;

    // can be: 0.045, 0.02, 0.01 etc
    //float m_HalfCellHeight = 0.02f;
    //float m_WallThickness = 0.01f;

	uint16_t m_MazeWidth = 0, m_MazeHeight = 0;
	uint32_t m_MazeArea, m_CellsAcrossWidth = 0, m_CellsAcrossHeight = 0, m_VisitedCellCount = 0;
};