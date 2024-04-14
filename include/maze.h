#pragma once

#include <map>

class Maze
{
public:
    Maze(uint16_t width, uint16_t height)
        : m_MazeWidth(width), m_MazeHeight(height) 
    {
        std::cout << "Maze Created" << std::endl;
    }

    Maze() = delete;
    Maze(Maze&) = delete;
    Maze(Maze&&) = delete;

	~Maze()
	{
		std::cout << "Maze Destroyed" << std::endl;

        ClearMazeVariables();
	}

	uint32_t DrawMaze(std::vector<std::pair<float, float>>& vertices, std::vector<uint32_t>& indices, uint32_t stackTop)
	{
        auto colorOfVertex = [&]()
        {
            // RG, BA color of each block
            if (m_CellCount == stackTop)
            {
                vertices.push_back(std::make_pair<float, float>(0.0f, 1.0f));
                vertices.push_back(std::make_pair<float, float>(0.0f, 1.0f));
            }
            else if (m_VisitedCellInfo.find(m_CellCount) != m_VisitedCellInfo.end() && m_VisitedCellInfo[m_CellCount] & CELL_VISITED)
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

        // This goes from bottomRight (-1, -1) to topLeft (1, 1)
        // WIDTH
        for (float i = -1.0f; i < 1.0f; i += 2 * m_HalfCellHeight + m_WallThickness)
        {
            // HEIGHT
            for (float j = -1.0f; j < 1.0f; j += 2 * m_HalfCellHeight + m_WallThickness)
            {
                bool isWallEast = true;
                if (m_VisitedCellInfo.find(m_CellCount) != m_VisitedCellInfo.end() && m_VisitedCellInfo[m_CellCount] & CELL_EAST)
                    isWallEast = false;

                bool isWallSouth = true;
                if (m_VisitedCellInfo.find(m_CellCount) != m_VisitedCellInfo.end() && m_VisitedCellInfo[m_CellCount] & CELL_SOUTH)
                    isWallSouth = false;

                indices.push_back((4 * m_CellCount) + 0);
                indices.push_back((4 * m_CellCount) + 1);
                indices.push_back((4 * m_CellCount) + 3);
                indices.push_back((4 * m_CellCount) + 1);
                indices.push_back((4 * m_CellCount) + 2);
                indices.push_back((4 * m_CellCount) + 3);

                std::vector<std::pair<float, float>> m_CellOrigin;
                m_CellOrigin.push_back(std::make_pair<float, float>(i + m_HalfCellHeight, j + m_HalfCellHeight));

                // right top
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first + m_HalfCellHeight + (isWallEast ? 0 : m_WallThickness), m_CellOrigin.back().second + m_HalfCellHeight));
                colorOfVertex();
                // right bottom
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first + m_HalfCellHeight + (isWallEast ? 0 : m_WallThickness), m_CellOrigin.back().second - m_HalfCellHeight - (isWallSouth ? 0 : m_WallThickness)));
                colorOfVertex();
                // left bottom
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first - m_HalfCellHeight, m_CellOrigin.back().second - m_HalfCellHeight - (isWallSouth ? 0 : m_WallThickness)));
                colorOfVertex();
                // left top
                vertices.push_back(std::make_pair<float, float>(m_CellOrigin.back().first - m_HalfCellHeight, m_CellOrigin.back().second + m_HalfCellHeight));
                colorOfVertex();

                if (m_VisitedCellInfo.find(m_CellCount) == m_VisitedCellInfo.end())
                {
                    m_VisitedCellInfo[m_CellCount] = 0x00;
                }

                m_CellCount++;
            }
        }

        return m_CellCount;
	}

    void ClearMazeVariables()
    {
        m_CellCount = 0;
    }

    bool MazeCompleted()
    {
        uint16_t cellsInOneAxis = static_cast<uint16_t>(2 / (2 * m_HalfCellHeight + m_WallThickness));
        // Checks if last element exists
        return m_VisitedCellCount == cellsInOneAxis * cellsInOneAxis;
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

    // can be: 0.045, 0.02, 0.01 etc
    float m_HalfCellHeight = 0.02f;
    float m_WallThickness = 0.01f;

	uint16_t m_MazeWidth = 0, m_MazeHeight = 0;
	uint32_t m_CellCount = 0;
    uint32_t m_VisitedCellCount = 0;

};